/*Kalman Filter, ported in Javascript by François Volral 2015 */

/*(The MIT License.)

Copyright (c) 2009, Kevin Lacker.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.*/



autowatch = 1;
outlets   = 2; 
setoutletassist(1, "Debug output");

//Global Var
var kalmanA;
var kalman_smooth_fact = 10.;

//Kalman Filter Object
var KalmanFilter = function () {
	this.timestep = null;
	this.state_dimension = null;
	this.observation_dimension = null;

	/* This group of matrices must be specified by the user. */

	/* F_k */
	this.state_transition = [];
	/* H_k */
	this.observation_model = [];
	/* Q_k */
	this.process_noise_covariance = [];
	/* R_k */
	this.observation_noise_covariance = [];

	/* The observation is modified by the user before every time step. */
	/* z_k */
	this.observation = [];

	/* This group of matrices are updated every time step by the filter. */
	/* x-hat_k|k-1 */
	this.predicted_state = [];
	/* P_k|k-1 */
	this.predicted_estimate_covariance = [];
	/* y-tilde_k */
	this.innovation = [];
	/* S_k */
	this.innovation_covariance = [];
	/* S_k^-1 */
	this.inverse_innovation_covariance = [];
	/* K_k */
	this.optimal_gain = [];
	/* x-hat_k|k */
	this.state_estimate = [];
	/* P_k|k */
	this.estimate_covariance = [];

	/* This group is used for meaningless intermediate calculations */
	this.vertical_scratch     = [];
	this.small_square_scratch = [];
	this.big_square_scratch   = [];

	this.initFilter = function(state_dimension, observation_dimension) {
		this.timestep = 0;
		this.state_dimension               = state_dimension;
		this.observation_dimension         = observation_dimension;
		this.state_transition              = matrix(state_dimension      , state_dimension);
		this.observation_model             = matrix(observation_dimension, state_dimension);
		this.process_noise_covariance      = matrix(state_dimension      , state_dimension);
		this.observation_noise_covariance  = matrix(observation_dimension, observation_dimension);
		this.observation                   = matrix(observation_dimension, 1);
		this.predicted_state               = matrix(state_dimension      , 1);
		this.predicted_estimate_covariance = matrix(state_dimension      , state_dimension);
		this.innovation                    = matrix(observation_dimension, 1);
		this.innovation_covariance         = matrix(observation_dimension, observation_dimension);
		this.inverse_innovation_covariance = matrix(observation_dimension, observation_dimension);
		this.optimal_gain                  = matrix(state_dimension      , observation_dimension);
		this.state_estimate                = matrix(state_dimension      , 1);
		this.estimate_covariance           = matrix(state_dimension      , state_dimension);
		this.vertical_scratch              = matrix(state_dimension      , observation_dimension);
		this.small_square_scratch          = matrix(observation_dimension, observation_dimension);
		this.big_square_scratch            = matrix(state_dimension      , state_dimension);	
	}
}


// In Out function 

function s_smooth(v) 
{ 
	kalmanA.observation_noise_covariance[0][0] = v;
	kalman_smooth_fact = v;
}

function init() 
{
	kalmanA = new KalmanFilter();
	kalmanA.initFilter(2,1);
	
	kalmanA.state_transition[0] = [ 1.0, 1.0];
	kalmanA.state_transition[1] = [ 0.0, 1.0];
	
	
	kalmanA.observation_model[0]   = [1.0, 0.0];

	/* The covariance matrices are blind guesses */
	set_identity_matrix(kalmanA.process_noise_covariance);
	set_identity_matrix(kalmanA.observation_noise_covariance);
	kalmanA.observation_noise_covariance[0][0] =  kalman_smooth_fact;
	
	/* Our knowledge of the start position is incorrect and unconfident */	
	kalmanA.state_estimate[0][0] = 10000;  
	kalmanA.state_estimate[1][0] = 0.0;
	
	set_identity_matrix(kalmanA.estimate_covariance);
	scale_matrix(kalmanA.estimate_covariance, 1000000);
}

function list(l) 
{
	var tbl = [];
    l = arrayfromargs(messagename,arguments);
	
	for (var i = 0; i < l.length; ++i) {
		kalmanA.observation[0][0] =  l[i];
		update(kalmanA);
		tbl[i] = kalmanA.state_estimate[0][0];
	}
	outlet(0, tbl);
}

//Kalman Functions
function update(f) 
{
  predict(f);
  estimate(f);
}

function predict(f) 
{
  f.timestep++;

  /* Predict the state */
	
  f.predicted_state = multiply_matrix(f.state_transition, f.state_estimate);

  /* Predict the state estimate covariance */
  f.big_square_scratch = multiply_matrix(f.state_transition, f.estimate_covariance);
  f.predicted_estimate_covariance = multiply_by_transpose_matrix(f.big_square_scratch, f.state_transition);
  f.predicted_estimate_covariance = add_matrix(f.predicted_estimate_covariance, f.process_noise_covariance);
}

function estimate(f) 
{
  /* Calculate innovation */
  f.innovation = multiply_matrix(f.observation_model, f.predicted_state);
  f.innovation = subtract_matrix(f.observation, f.innovation);

  /* Calculate innovation covariance */
  
  f.vertical_scratch = multiply_by_transpose_matrix(f.predicted_estimate_covariance, f.observation_model);
  
  f.innovation_covariance = multiply_matrix(f.observation_model, f.vertical_scratch);
  f.innovation_covariance = add_matrix(f.innovation_covariance, f.observation_noise_covariance);

  /* Invert the innovation covariance.
     Note: this destroys the innovation covariance.
     TODO: handle inversion failure intelligently. */
   destructive_invert_matrix(f.innovation_covariance, f.inverse_innovation_covariance);
  
  /* Calculate the optimal Kalman gain.
     Note we still have a useful partial product in vertical scratch
     from the innovation covariance. */
  f.optimal_gain = multiply_matrix(f.vertical_scratch, f.inverse_innovation_covariance);

  /* Estimate the state */
  f.state_estimate = multiply_matrix(f.optimal_gain, f.innovation);
  f.state_estimate = add_matrix(f.state_estimate, f.predicted_state);

  /* Estimate the state covariance */
  f.big_square_scratch = multiply_matrix(f.optimal_gain, f.observation_model);
  subtract_from_identity_matrix(f.big_square_scratch);
  f.estimate_covariance = multiply_matrix(f.big_square_scratch, f.predicted_estimate_covariance);
}

// Matrix functions little library

function matrix(r, c) 
{ 
	// return a two dimensions array
	var tbl = [];
	tbl.length = r;
	
	for (var i = 0; i < r; i++) {
		tbl[i] = [];
		tbl[i].length = c;
	}
	
	for (var i = 0 ; i < tbl.length; i++) {
		for (var j = 0 ; j < tbl[0].length; j++) {
			tbl[i][j] = 0.0;
		}
	}
	return tbl;
}

function add_matrix(a, b) 
{
	var tbl;
	if (a.length > 0 && b.length > 0) {
		if (a[0].length > 0 && b[0].length > 0) {
			if (a.length == b.length && a[0].length == b[0].length) {
				tbl = matrix(a.length, a[0].length);
			} else {
				outlet(1, "add_matrix : Error a and b matrix doesn't have the same length");
				return;
			}	
		} else {
			outlet(1, "add_matrix : Error a[0] or b[0] matrix is empty");
			return;
		}	
	} else {
		outlet(1, "add_matrix : Error a or b matrix is empty");
		return;
	}
	
	for (var i = 0; i < a.length; ++i) {
		for (var j = 0; j < a[0].length; ++j) {
			tbl[i][j] = a[i][j] + b[i][j];
		}
	}
	
	return tbl;
}

function subtract_matrix(a, b) 
{
  var tbl;
  
	if (a.length > 0 && b.length > 0) {
		if (a[0].length > 0 && b[0].length > 0) {
			if (a.length == b.length && a[0].length == b[0].length) {
				tbl = matrix(a.length, a[0].length);
			} else {
				outlet(1, "subtract_matrix : Error a and b matrix doesn't have the same length");
				return;
			}	
		} else {
			outlet(1, "subtract_matrix : Error a[0] or b[0] matrix is empty");
			return;
		}	
	} else {
		outlet(1, "subtract_matrix : Error a or b matrix is empty");
		return;
	}
	
  for (var i = 0; i < a.length; ++i) {
    for (var j = 0; j < a[0].length; ++j) {
      tbl[i][j] = a[i][j] - b[i][j];
    }
  }
  return tbl;
}

function subtract_from_identity_matrix(a) 
{
  // Square Matrix Only
  for (var i = 0; i < a.length; ++i) {
    for (var j = 0; j < a[0].length; ++j) {
      if (i == j) {
		a[i][j] = 1.0 - a[i][j];
      } else {
		a[i][j] = 0.0 - a[i][j];
      }
    }
  }
}

function multiply_matrix(a, b) 
{
	var tbl;
	tbl = matrix(a.length, b[0].length);

	for (var i = 0; i < tbl.length; ++i) {
		for (var j = 0; j < tbl[0].length; ++j) {
			// Calculate element c.data[i][j] via a dot product of one row of a with one column of b
			tbl[i][j] = 0.0;
			for (var k = 0; k < a[0].length; ++k) {
				tbl[i][j] += a[i][k] * b[k][j];
			}
		}
	}
	return tbl;
}

function multiply_by_transpose_matrix(a, b) 
{
	var tbl;
	tbl = matrix(a.length, b.length);

	for (var i = 0; i < tbl.length; ++i) {
		for (var j = 0; j < tbl[0].length; ++j) {
			// Calculate element c.data[i][j] via a dot product of one row of a with one column of b
			tbl[i][j] = 0.0;
			for (var k = 0; k < a[0].length; ++k) {
				tbl[i][j] += a[i][k] * b[j][k];
			}
		}
	}
	return tbl;
}

function destructive_invert_matrix(input, output) 
{
	/* Uses Gauss-Jordan elimination.

   The elimination procedure works by applying elementary row
   operations to our input matrix until the input matrix is reduced to
   the identity matrix.
   Simultaneously, we apply the same elementary row operations to a
   separate identity matrix to produce the inverse matrix.
   If this makes no sense, read wikipedia on Gauss-Jordan elimination.
   
   This is not the fastest way to invert matrices, so this is quite
   possibly the bottleneck. */
	
	/* Convert input to the identity matrix via elementary row operations.
	The ith pass through this loop turns the element at i,i to a 1
	and turns all other elements in column i to a 0. */

	set_identity_matrix(output);
	
	for (var i = 0; i < input.length; ++i) {		
		if (input[i][i] == 0.0) {
			/* We must swap rows to get a nonzero diagonal element. */
			var r;
			for (r = i + 1; r < input.length; ++r) {
				if (input[r][i] != 0.0) {
					break;
				}
			}

			if (r == input.length) { 
				outlet(1, "Matrix can't be inverted");
				return ;
			}
			
			swap_rows(input, i, r);
			swap_rows(output, i, r);
		}

		/* Scale this row to ensure a 1 along the diagonal. We might need to worry about overflow from a huge scalar here. */
		var scalar = 1.0 / input[i][i];
		
		scale_row(input, i, scalar);
		scale_row(output, i, scalar);

		/* Zero out the other elements in this column. */
		for (var j = 0; j < input.length; ++j) {
			if (i == j) {
				continue;
			}
			var shear_needed = -input[j][i];
			shear_row(input , j, i, shear_needed);
			shear_row(output, j, i, shear_needed);
		}
	}
	return ;
}

function set_identity_matrix(m) 
{
  for (var i = 0; i < m.length; ++i) {
    for (var j = 0; j < m[0].length; ++j) {
      if (i == j) {
		m[i][j] = 1.0;
      } else {
		m[i][j] = 0.0;
      }
    }
  }
}

function scale_matrix(m, scalar) 
{ 
  for (var i = 0; i < m.length; ++i) {
    for (var j = 0; j < m[0].length; ++j) {
      m[i][j] *= scalar;
    }
  }
}



function swap_rows(m, r1, r2) 
{
  var tmp = m[r1];
  m[r1] = m[r2];
  m[r2] = tmp;
}

function shear_row(m, r1, r2, scalar) 
{
  /* Add scalar * row r2 to row r1. */
  //assert(r1 != r2);
  for (var i = 0; i < m[0].length; ++i) {
    m[r1][i] += scalar * m[r2][i];
  }
}

function scale_row(m, r, scalar) 
{
  //Scalar can't be 0
  for (var i = 0; i < m[0].length; ++i) {
    m[r][i] *= scalar;
  }
}

//Some functions about matrix useful for debug :
function info_matrix(name, f) 
{
	//Used for display
	outlet(1, "Size of " + name + " is " + f.length + "x" + f[0].length);
}

function fix_matrix(m) 
{
	//Used for display, fix the number of decimal place
	for (var i = 0; i < m.length; ++i) {
		for (var j = 0; j < m[0].length; ++j) {
			m[i][j] = parseFloat(Math.round(m[i][j] * 100) / 100).toFixed(2);;
		}
	}
}

function out_matrix(mat) 
{
	//Used for diplay
	a = "";
	for(var i = 0; i < mat.length; i++) {
		a = "";
		for (var j = 0; j < mat[0].length ; j++) {
			a = a + mat[i][j] + " ";
		}
		outlet(1, a);
	}
}
