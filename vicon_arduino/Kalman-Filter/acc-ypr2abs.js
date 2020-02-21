inlets = 2 // acc rmatrix
outlets = 1 // S

var deltaT = 0.1;
var acc = matrix(1,3);
var rmatrix = matrix(3,3);
var S = [];
var V = [];
var aWorld = [];

	
function bang()
{
 	S = [0,0,0];
 	V = [0,0,0];
}

function list()
{
	var l = arrayfromargs(arguments);
	if (inlet == 0) {
		for (var i = 0; i < 3; i++)
			acc[0][i] = l[i];
	} else if (inlet == 1) {
		for (var i = 0; i < 9; i++) {
			var j = Math.floor(i/3);
			rmatrix[j][i%3] = l[i];
		}
	}
	aWorld = multiply_matrix(acc, rmatrix);
	ec14();
//	ec13();
//	out_matrix(0, aWorld);
	outlet(0, S);
}

function ec14() {	// update V and S
	for (var i = 0; i < 3; i++) {
		V[i] += aWorld[0][i] * deltaT;
		S[i] += V[i] * deltaT;
	}
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
	//outlet(1, a.length+" "+a[0].length+" b: "+b.length+" "+b[0].length);
	tbl = matrix(a.length, b[0].length);

	for (var i = 0; i < tbl.length; ++i) {
		for (var j = 0; j < tbl[0].length; ++j) {
			// Calculate element c.data[i][j] via a dot product of one row of a with one column of b
			tbl[i][j] = 0.0;
			for (var k = 0; k < a[0].length; k++) {
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

function out_matrix(which, mat) 
{
	//Used for diplay
	a = "";
	for(var i = 0; i < mat.length; i++) {
		a = "";
		for (var j = 0; j < mat[0].length ; j++) {
			a = a + mat[i][j] + " ";
		}
		outlet(which, a);
	}
}
