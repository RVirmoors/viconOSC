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

	// initial covariances of state noise, measurement noise
	double P0 = 1;
	double R0 = 1;

	int i, j;

	for (i = 0; i < 3; ++i) {
		ekf->P[i][i] = P0;
		ekf->Q[i][i] = 1;
		for (j = 0; j < 3; ++j) {
			ekf->Rbw[i][j] = 0;
		}
	}

	for (i = 0; i < 9; ++i)
		ekf->R[i][i] = R0;

	// position
	ekf->x[0] = 0;
	ekf->x[1] = 0;
	ekf->x[2] = 0;
	ekf->xB[0] = 0;
	ekf->xB[1] = 0;
	ekf->xB[2] = 0;
}

static void model(ekf_t * ekf, double SV[Mobs], double SV_Rho[Nsta]) // vicon, acc, angles
{
	int i, j;

	// state model
	for (i = 0; i < Nsta; i++) {
		ekf->x[i] = ekf->x[i] + ekf->Q[i][i];
	}

	// meas model
	for (i = 0; i < Nsta; i++) {
		SV_Rho[i] = SV[i] + ekf->R[i][i];
	}

	for (j = 0; j < Nsta; j++) {
		ekf->fx[j] = ekf->x[j];
	}

	for (j = 0; j < Nsta; ++j)
		ekf->F[j][j] = 1;

	for (i = 0; i < Mobs; ++i) {
		ekf->hx[i] = 0;
	}

	for (i = 0; i < Nsta; ++i) {
		ekf->H[i][i] = 1;
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
double SV_Pos[Mobs];
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
	if (argc != (Mobs)) {
		object_error((t_object *)x, "wrong number of params! Need %d observations.", Mobs);
	}
	else {
		// get position measurements
		for (int i = 0; i < Mobs; i++) {
			SV_Pos[i] = atom_getfloat(&argv[i]);
		}

		model(&ekf, SV_Pos, SV_Rho);
		ekf_step(&ekf, SV_Rho);  // sv_rho is the output of the meas. model

		t_atom result[3];
		// get result from EKF output
		for (int k = 0; k < 3; ++k)
			atom_setfloat(&result[k], ekf.x[k]);
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


