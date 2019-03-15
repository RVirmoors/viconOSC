/**
	@file
	kalman - extended kalman filter

	@ingroup	examples
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
#include "tinyekf_cfg.h"
#include "tiny_ekf.h"

// positioning interval
static const double T = 1;

static void blkfill(ekf_t * ekf, const double * a, int off)
{
	off *= 2;

	ekf->Q[off][off] = a[0];
	ekf->Q[off][off + 1] = a[1];
	ekf->Q[off + 1][off] = a[2];
	ekf->Q[off + 1][off + 1] = a[3];
}


static void init(ekf_t * ekf)
{
	// Set Q, see [1]
	const double Sf = 36;
	const double Sg = 0.01;
	const double sigma = 5;         // state transition variance
	const double Qb[4] = { Sf*T + Sg * T*T*T / 3, Sg*T*T / 2, Sg*T*T / 2, Sg*T };
	const double Qxyz[4] = { sigma*sigma*T*T*T / 3, sigma*sigma*T*T / 2, sigma*sigma*T*T / 2, sigma*sigma*T };

	blkfill(ekf, Qxyz, 0);
	blkfill(ekf, Qxyz, 1);
	blkfill(ekf, Qxyz, 2);
	blkfill(ekf, Qb, 3);

	// initial covariances of state noise, measurement noise
	double P0 = 10;
	double R0 = 36;

	int i;

	for (i = 0; i < 8; ++i)
		ekf->P[i][i] = P0;

	for (i = 0; i < 9; ++i)
		ekf->R[i][i] = R0;

	// position
	ekf->x[0] = 0;
	ekf->x[2] = 0;
	ekf->x[4] = 0;

	// velocity
	ekf->x[1] = 0;
	ekf->x[3] = 0;
	ekf->x[5] = 0;

	// clock bias
	ekf->x[6] = 0;

	// clock drift
	ekf->x[7] = 0;
}

static void model(ekf_t * ekf, double SV[Mobs][3])
{

	int i, j;

	for (j = 0; j < 8; j += 2) {
		ekf->fx[j] = ekf->x[j] + T * ekf->x[j + 1];
		ekf->fx[j + 1] = ekf->x[j + 1];
	}

	for (j = 0; j < 8; ++j)
		ekf->F[j][j] = 1;

	for (j = 0; j < 4; ++j)
		ekf->F[2 * j][2 * j + 1] = T;

	double dx[9][3];

	for (i = 0; i < 9; ++i) {
		ekf->hx[i] = 0;
		for (j = 0; j < 3; ++j) {
			double d = ekf->fx[j * 2] - SV[i][j];
			dx[i][j] = d;
			ekf->hx[i] += d * d;
		}
		ekf->hx[i] = pow(ekf->hx[i], 0.5) + ekf->fx[6];
	}

	for (i = 0; i < 9; ++i) {
		for (j = 0; j < 3; ++j)
			ekf->H[i][j * 2] = dx[i][j] / ekf->hx[i];
		ekf->H[i][6] = 1;
	}
}

////////////////////////// object struct
typedef struct _kalman
{
	t_object					ob;			// the object itself (must be first)
	void *m_outlet1;
} t_kalman;

///////////////////////// function prototypes
//// standard set
void *kalman_new(t_symbol *s, long argc, t_atom *argv);
void kalman_free(t_kalman *x);
void kalman_assist(t_kalman *x, void *b, long m, long a, char *s);
void kalman_input(t_symbol *s, long argc, t_atom *argv);

//////////////////////// global class pointer variable
void *kalman_class;

ekf_t ekf;
// Make a place to store the data from the inlet and the output of the EKF
double SV_Pos[Mobs][3];
double SV_Rho[Mobs]; //is zero

void ext_main(void *r)
{
	t_class *c;

	c = class_new("kalman", (method)kalman_new, (method)kalman_free, (long)sizeof(t_kalman),
				  0L /* leave NULL!! */, A_GIMME, 0);

	/* you CAN'T call this from the patcher */
	class_addmethod(c, (method)kalman_assist,			"assist",		A_CANT, 0);
	class_addmethod(c, (method)kalman_input,			"pos",		A_GIMME, 0);

	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	kalman_class = c;

	post("Initializing EKF object");

	// Do generic EKF initialization

	ekf_init(&ekf, Nsta, Mobs);

	// Do local initialization
	init(&ekf);
}

void kalman_assist(t_kalman *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		sprintf(s, "I am inlet %ld", a);
	}
	else {	// outlet
		sprintf(s, "I am outlet %ld", a);
	}
}

void kalman_free(t_kalman *x)
{
	;
}

void kalman_input(t_kalman *x, t_symbol *s, long argc, t_atom *argv) {
	if (argc != (Mobs * 3)) {
		object_error((t_object *)x, "wrong number of params! Need %d observations.", Mobs);
	}
	else {
		// get position measurements
		for (int i = 0; i < Mobs; i++) {
			for (int j = 0; j < 3; ++j) {
				SV_Pos[i][j] = atom_getfloat(&argv[i*3 + j]);
			}
		}

		model(&ekf, SV_Pos);
		ekf_step(&ekf, SV_Rho);

		t_atom result[3];
		// get result from EKF output
		for (int k = 0; k < 3; ++k)
			atom_setfloat(&result[k], ekf.x[2 * k]);
		// print result to outlet
		outlet_list(x->m_outlet1, 0L, 3, result);
	}
}


void *kalman_new(t_symbol *s, long argc, t_atom *argv)
{
	t_kalman *x = NULL;
	long i;

	if ((x = (t_kalman *)object_alloc(kalman_class))) {
	//	object_post((t_object *)x, "a new %s object was instantiated: %p", s->s_name, x);
	//	object_post((t_object *)x, "it has %ld arguments", argc);

		for (i = 0; i < argc; i++) {
			if ((argv + i)->a_type == A_FLOAT) {
				object_post((t_object *)x, "arg %ld: float (%f)", i, atom_getfloat(argv+i));
			} else {
				object_error((t_object *)x, "forbidden argument");
			}
		}
	}

	x->m_outlet1 = listout((t_object *)x);
	return (x);
}


