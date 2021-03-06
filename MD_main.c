/*
  MD_main.c

  Created by Anders Lindman on 2013-10-31.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "initfcc.h"
#include "alpotential.h"

#define nbr_of_timesteps 1023
#define N 256 // number of atoms
#define a0 1 // lattice parameter
#define m 27 // aluminum atom mass

/* Main program */
int main()
{

	int i,j;

	double pos[N][3][nbr_of_timesteps+1], vel[N][3], acc[N][3], pos_tmp[N][3],f[N][3] ;

	double current_time;

	FILE *file_energy;

	/* energies */
	double *PE = malloc((nbr_of_timesteps+1) * sizeof (double));
	double *KE = malloc((nbr_of_timesteps+1) * sizeof (double));
	double *E_tot = malloc((nbr_of_timesteps+1) * sizeof (double));
/*
  Function that generates a fcc lattice in units of [Å]. Nc is the number of
  primitive cells in each direction and a0 is the lattice parameter. The
  positions of all the atoms are stored in pos which should be a matrix of the
  size N x 3, where N is the number of atoms. The first, second and third column
  correspond to the x,y and z coordinate respectively.
*/

        double Nc=pow(N/4.0,1/3.0);
	init_fcc(pos_tmp, (int)round(Nc), a0);

//	printf("%e%s",pos[60][2],"\n");

/* Perturb the atom positions randomly */

	srand(time(NULL));
	double random_value;
	random_value = 2 * (double) rand() / (double) RAND_MAX - 1;

	//printf("%e%s",random_value,"\n");

	for (i = 0; i < N -1; i++){
		for (j = 0; j < 3; j++){
			// random value in interval [-1,1]
			random_value = 2 * (double) rand() / (double) RAND_MAX - 1;
			//printf("%e%s",pos[i][j],"\n");
			pos_tmp[i][j] = pos_tmp[i][j] + 0.05*a0*random_value;
			//printf("%e%s",pos[i][j],"new \n");

		}
	}


/* TIME-STEPPING */

	double timestep = 1;

	/* Set initial velocites */
	for (i = 0; i < N-1; i++) {
		vel[i][0] = 0;
		vel[i][1] = 0;
		vel[i][2] = 0;
	}
	/* Set initial positions */
	for (i = 0; i < N-1; i++) {
		pos[i][0][0] = pos_tmp[i][0];
		pos[i][1][0] = 	pos_tmp[i][1];
		pos[i][2][0] = 	pos_tmp[i][2];
	}

	/* Calculate initial accelerations based on initial displacements */

	/*
	  Function that calculates the potential energy in units of [eV]. pos should be
	  a matrix containing the positions of all the atoms, L is the length of the
	  supercell and N is the number of atoms.
	*/
        /* Calculate initial energy based on initial conditions */
	PE[0] = get_energy_AL(pos_tmp, a0, N);
	for (j = 0; j < N-1; j++) {
	        KE[j] = 0;
	}
	E_tot[0] = PE[0]+KE[0];

	/* initial acceleration */
	for (j = 0; j < N-1; j++) {
	get_forces_AL(f,pos_tmp, a0, N);
	        acc[j][0] = f[j][0]/m;
		acc[j][1] = f[j][1]/m;
		acc[j][2] = f[j][2]/m;
		//	printf("%g%s",acc[j][1],"\n");
	}
	/* timesteps according to velocity Verlet algorithm */
	for (i = 1; i < nbr_of_timesteps + 1; i++) {
		/* v(t+dt/2) */
		for (j = 0; j < N-1; j++) {
			vel[j][0] += timestep * 0.5 * acc[j][0];
			vel[j][1] += timestep * 0.5 * acc[j][1];
			vel[j][2] += timestep * 0.5 * acc[j][2];
			//	printf("%g%s",vel[j][1],"\n");
		}

		/* q(t+dt) */
		for (j = 0; j < N-1; j++) {
			pos_tmp[j][0] += timestep * vel[j][0];
			pos_tmp[j][1] += timestep * vel[j][1];
			pos_tmp[j][2] += timestep * vel[j][2];
		}

		/* a(t+dt) */
		for (j = 0; j < N-1; j++) {
		get_forces_AL(f,pos_tmp, a0, N);
	        acc[j][0] = f[j][0]/m;
		acc[j][1] = f[j][1]/m;
		acc[j][2] = f[j][2]/m;
		}
		//	printf("%g%s",acc[j][1],"\n");
		/* v(t+dt) */
		for (j = 0; j < N -1; j++) {
			vel[j][0] += timestep * 0.5 * acc[j][0];
			vel[j][1] += timestep * 0.5 * acc[j][1];
			vel[j][2] += timestep * 0.5 * acc[j][2];

			KE[i] = KE[i] + m*vel[j][0]*vel[j][0]/2;
			KE[i] = KE[i] + m*vel[j][1]*vel[j][1]/2;
			KE[i] = KE[i] + m*vel[j][2]*vel[j][2]/2;
		}

		/* Save the atom positions */
	        pos[j][0][i] = pos_tmp[j][0];
		pos[j][1][i] = pos_tmp[j][1];
		pos[j][2][i] = pos_tmp[j][2];

		/* Save the energy */
	    	PE[i] = get_energy_AL(pos_tmp, a0, N);
	       	E_tot[i] = PE[i] + KE[i];
	}
	/* Print energy data to output file */
	file_energy = fopen("energy.dat","w");

	for (i = 0; i < nbr_of_timesteps + 1; i++) {
		current_time = i * timestep;
		fprintf(file_energy, "%.4f \t %e \t %e \t %e", current_time, E_tot[i], PE[i], KE[i] );
		fprintf(file_energy, "\n");
	}
	fclose(file_energy);

}
