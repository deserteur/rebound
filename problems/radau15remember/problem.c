/**
 * @file 	problem.c
 * @brief 	Example problem: self-gravity disc.
 * @author 	Hanno Rein <hanno@hanno-rein.de>
 * @detail 	A self-gravitating disc is integrated using
 * the leap frog integrator. This example is also compatible with 
 * the Wisdom Holman integrator. Collisions are not resolved.
 * 
 * @section 	LICENSE
 * Copyright (c) 2011 Hanno Rein, Shangfei Liu
 *
 * This file is part of rebound.
 *
 * rebound is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rebound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rebound.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <gmp.h>
#include <time.h>
#include <sys/time.h>
#include "main.h"
#include "particle.h"
#include "boundaries.h"
#include "output.h"
#include "communication_mpi.h"
#include "tree.h"
#include "input.h"
#include "tools.h"

double energy();
double velocity();
double energy_init = 0; 
extern double 	integrator_epsilon;
extern double 	integrator_min_dt;

void problem_init(int argc, char* argv[]){
	// Setup constants
	G 		= 1;		

	// Setup homog. sphere
	tmax = 1;
	dt		= 1;//tmax/input_get_double(argc,argv,"timesteps",1000);
	integrator_epsilon = input_get_double(argc,argv,"epsilon",0.);
	integrator_min_dt = 1e-10;
	exit_simulation=1;
	boxsize = 5;	
	init_box();


	// test particle 
	struct particle p1;
	p1.x = 0.3; 
	p1.y = 0.5; 
	p1.z = 0; 
	p1.vx = 0; 
	p1.vy = -1.; 
	p1.vz = 0; 
	p1.m = 100;
	particles_add(p1);
	
	struct particle p2;
	p2.x = 0; 
	p2.y = 0; 
	p2.z = 0; 
	p2.vx = 0; 
	p2.vy = 0; 
	p2.vz = 0; 
	p2.m = 100;
	particles_add(p2);

	struct particle p3;
	p3.x = -0.3; 
	p3.y = -0.5; 
	p3.z = 0; 
	p3.vx = -.4; 
	p3.vy = 0; 
	p3.vz = 0; 
	p3.m = 100;
	particles_add(p3);

	mpf_set_default_prec(512);
	energy_init = energy();
}


double energy(){
	mpf_t energy_kinetic;
	mpf_init(energy_kinetic);
	mpf_t energy_potential;
	mpf_init(energy_potential);
	for (int i=0;i<N;i++){
		for (int j=0;j<i;j++){
			double dx = particles[i].x - particles[j].x;
			double dy = particles[i].y - particles[j].y;
			double dz = particles[i].z - particles[j].z;
			double r = sqrt(dx*dx + dy*dy + dz*dz + softening*softening);
			mpf_t temp;
			mpf_init(temp);
			mpf_set_d(temp,-G*particles[j].m/r);
			mpf_add(energy_potential, energy_potential,temp);
			
		}
		double dvx = particles[i].vx;
		double dvy = particles[i].vy;
		double dvz = particles[i].vz;
		mpf_t temp;
		mpf_init(temp);
		mpf_set_d(temp,1./2. * (dvx*dvx + dvy*dvy + dvz*dvz));
		mpf_add(energy_kinetic, energy_kinetic,temp);
	}
	mpf_add(energy_kinetic,energy_kinetic,energy_potential);

	return mpf_get_d(energy_kinetic);
}
double velocity(){
	double dvx = particles[0].vx;
	double dvy = particles[0].vy;
	double dvz = particles[0].vz;
	return sqrt(dvx*dvx + dvy*dvy + dvz*dvz);
}

void problem_inloop(){
	printf ("\ntimestep %f %f \n",t,dt);
}

void problem_finish(){
	printf("\nFinished. Time: %e. Difference to exact time: %e. Timestep: %e\n",t,t-tmax,dt);
#ifdef INTEGRATOR_LEAPFROG
	FILE* of = fopen("energy_leapfrog.txt","a+"); 
#endif
#ifdef INTEGRATOR_RADAU15
	FILE* of = fopen("energy_radau15.txt","a+"); 
#endif
	fprintf(of,"%e\t",dt);
	fprintf(of,"%e\t",integrator_epsilon);
	fprintf(of,"%e\t",fabs((energy()-energy_init)/energy_init));
	fprintf(of,"\n");
	fclose(of);
}

void problem_output(){
}

