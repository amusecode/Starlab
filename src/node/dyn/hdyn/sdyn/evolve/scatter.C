//// modified scatter program for multiple experiments
//// scatter:  simple N-body scattering program.
////
//// Options:      -A    specify accuracy parameter [0.02]
////               -c    specify CPU time check interval (s) [3600]
////               -C    specify cube size for snapshot output [10]
////               -d    specify log output interval [none]
////               -D    specify snapshot output interval [none]
////               -g    specify tidal tolerance [1.E-6]
////               -i    specify makescat-style string to initialize
////                        the integration ["-M 1 -v 2 -t -p"]
////               -n    specify the number of scattering experiments [1]
////               -p    read initial state from cin [false]
////               -t    specify time span of integration [infty]
////               -v    verbose mode [false]
////               -s    seed [system clock]
////                     Use negative value to prevent mkscat from
////                     reinitializing the random number sequence.

#include "scatter.h"
//#include "sdyn.h"
#include "sigma.h"

#ifndef TOOLBOX

ostream& operator<<(ostream& s, scatter_input& i) {
 
  s << "Scatter_input: " << endl;
  s << "Input string: " << i.init_string << endl;
  s << " i.dt = " << i.delta_t << " " << i.dt_out << " " << i.dt_snap << endl;
  s << " eta= " << i.eta << endl;
  s << " tol= " << i.tidal_tol_factor  << endl;
  s << " Cs= " << i.snap_cube_size << " dt cpu= " << i.cpu_time_check << endl;
  s << " e_exp= " << i.n_experiments << endl;
  s << " rnd= " << i.seed << " "<< i.n_rand <<" "<< i.pipe  << " "<< i.debug;

  return s;
}

void ppn(sdyn* b, ostream & s, int level) {

  s.precision(3);

  for (int i = 0; i < 2*level; i++) s << " ";

  if(b != b->get_root()) {
    b->pretty_print_node(s);
    cerr << endl;
    s << "   "<< b->get_mass() << "  "
      << b->get_pos() << " (r= " << abs(b->get_pos()) << ")  " 
      << b->get_vel() << " (v= " << abs(b->get_vel()) << ")" << endl;
    //	<< "r= " << abs(b->get_pos()) << "  " 
    //	<< "v= " << abs(b->get_vel()) << endl;
  }

  for (sdyn * daughter = b->get_oldest_daughter();
       daughter != NULL;
       daughter = daughter->get_younger_sister())
    ppn(daughter, s, level + 1);	
}

void scatter(sdyn* b, scatter_input input, 
	     scatter_exp &experiment) {

  scatter(b, input.eta,
	     input.delta_t, input.dt_out, input.cpu_time_check,
	     input.dt_snap, input.tidal_tol_factor, input.snap_cube_size,
	     input.debug,
	     experiment);

}

local real calculate_angular_mometum_from_scratch(sdyn* b)
{
    vec L = 0;
    for_all_leaves(sdyn, b, bi) {
	L += bi->get_mass()*(bi->get_pos()^bi->get_vel());
    }
    return abs(L);
}

#define DT_CHECK	20

void scatter(sdyn* b, real eta,
	     real delta_t, real dt_out, real cpu_time_check,
	     real dt_snap, real ttf, real snap_cube_size,
	     int debug,
	     scatter_exp &experiment) {

    experiment.init_scatter_exp(b); 

    real min_min_ssd = experiment.get_min_min_ssd();

    real de_merge = 0;
    int stop_at_cpu = 0;
    real cpu_save = cpu_time();
    bool terminate = false;

    real t_out = b->get_time_offset();
    real t_end = delta_t + (real)b->get_time_offset();	// xreal

    char previous_form[255];
    char current_form[255];
    strcpy(current_form, get_normal_form(b));

    real kin=0, pot=0;
    b->flatten_node();
    real etot_init = calculate_energy_from_scratch(b, kin, pot); 
    real Ltot_init = calculate_angular_mometum_from_scratch(b);
    make_tree(b, DYNAMICS, STABILITY, K_MAX, false);

    // Loop continues forever, or until termination time is reached, or
    // system is unbound (see extend_or_end.C).

    for (real t = 0; t <= VERY_LARGE_NUMBER; t += DT_CHECK) {

	terminate = tree_evolve(b,  DT_CHECK, dt_out, dt_snap, 
				snap_cube_size, 
				eta, min_min_ssd, cpu_time_check);

	make_tree(b, DYNAMICS, !STABILITY, K_MAX, false);
	calculate_energy(b, kin, pot);
	if (debug) {
	    cerr.precision(6), cerr << "\nStatus at time " << b->get_time();
	    cerr.precision(9), cerr << " (energy = " << kin + pot << "): ";
	    print_normal_form(b, cerr);
	    //      cerr << endl;
	    //      ppn(b, cerr);
	    //      vec center = b->get_pos();
	    //      print_structure_recursive(b, 0., center, true, true, 4);

	    cerr.precision(6);
	}
    
	// Check to see if the scattering is over.
	if (cpu_time_check < 0
	    && cpu_time() - cpu_save > abs(cpu_time_check)) {
	    cerr <<" tcpu>tcpu_check"<<endl;
	    return;
	}
    
	if (t>=dt_out) {
	    real ekin, epot;
	    calculate_energy(b, ekin, epot);
	    if(debug==1) {
		cerr << "Time = " << b->get_time() 
		     << "  Etot = " << ekin + epot 
		     << "  Tinf = " << ekin/epot << endl;
	    }
	    //      put_sdyn(b);
	    t_out += dt_out;
	}
    
	if (t>=t_end) {
	    cerr << "Early termination" << endl;
	    experiment.set_scatter_discriptor(stopped);
	    experiment.set_stop(true);
	    real ekin, epot;
	    calculate_energy(b, ekin, epot);
	    cerr << "Time = " << b->get_time() 
		 << "  Etot = " << ekin + epot << endl;

	    print_normal_form(b, cerr);
	    b->set_name("root");
	    ppn(b, cerr);
	    vec center = b->get_pos();
	    print_structure_recursive(b, 0., center, true, true, 4);
	    cerr << "Done" << endl;

	    break;
	}
    
	de_merge += merge_collisions(b);
    
	make_tree(b, DYNAMICS, STABILITY, K_MAX, false);
	//    make_tree(b, !DYNAMICS, STABILITY, K_MAX, true);
    
	int unbound = extend_or_end_scatter(b, ttf, false);
	if(unbound==2) {   // two body system is bound but we stop anyway
	    experiment.set_final_bound(true);
	    unbound = 1;   // but stop anyway
	}
	//bool unbound = tree_is_unbound(b, ttf, false);
	//make_tree(b, DYNAMICS, STABILITY, K_MAX, debug);
	if (unbound || terminate) break;
    
	//	if(experiment.get_form_changes()>1) 
	//	  cerr << "Resonant encounter: " << get_normal_form(b) << endl;
	strcpy(previous_form, current_form);
	strcpy(current_form, get_normal_form(b));
	if(strcmp(previous_form, current_form))
	    experiment.inc_form_changes();
    
	//	print_normal_form(b, cerr);
    }
  
    b->flatten_node();
    real etot_error = calculate_energy_from_scratch(b, kin, pot) 
	- etot_init - de_merge;
    experiment.set_energy_error(etot_error);
    real Ltot_error = (calculate_angular_mometum_from_scratch(b)
		       - Ltot_init)/Ltot_init;
    experiment.set_angular_momentum_error(Ltot_error);
    //PRL(etot_error);

    experiment.set_min_min_ssd(min_min_ssd);
    // here we want to initialize the 
    // experiment.set_....
    // information about minimal distance to next stellar surface
    // and information about which star is near to what other.
    // see scatter_exp.h for more details.
    // printing is done in scatter_MPI.C in the function master_process
    // near the location where min_min_ssd is printed.

    make_tree(b, DYNAMICS, STABILITY, K_MAX, debug);
  
    // add final report
    experiment.final_scatter_exp(b);
    //PRL(experiment.get_final_form());

}

#if 0

// scatter: Take the system with root node b and integrate it forward
// 	    in time up to time delta_t.  Check the state of the system
//	    every DT_CHECK time units.

void scatter(sdyn* b, scatter_input input, 
	     scatter_exp &experiment)
{
    real eta = input.eta;
    real delta_t = input.delta_t;
    real dt_out = input.dt_out;
    real cpu_time_check = input.cpu_time_check;
    real dt_snap = input.dt_snap;
    real ttf = input.tidal_tol_factor;
    real snap_cube_size = input.snap_cube_size;
    int debug = input.debug;
  
    experiment.init_scatter_exp(b); 
  
    real de_merge = 0;
    int stop_at_cpu = 0;
    real cpu_save = cpu_time();
    bool terminate = false;
  
    real t_out = b->get_time_offset();
    real t_end = delta_t + (real)b->get_time_offset(); // is xreal
    PRC(t_out);PRL(t_end); 
   
    char previous_form[255];
    char current_form[255];
    strcpy(current_form, get_normal_form(b));
   
    real kin=0, pot=0;
    b->flatten_node();
    real etot_init = calculate_energy_from_scratch(b, kin, pot); 
    make_tree(b, DYNAMICS, STABILITY, K_MAX, debug);
   
    for (real t = 0; t <= delta_t; t += DT_CHECK) {
	PRC(t);
	terminate = tree_evolve(b,  DT_CHECK, dt_out, dt_snap, 
				snap_cube_size, 
				eta, cpu_time_check);
   
	calculate_energy(b, kin, pot);
	if (debug) {
	    cerr.precision(6), cerr << "\nStatus at time " << b->get_time();
	    cerr.precision(9), cerr << " (energy = " << kin + pot << "):";
	    print_normal_form(b, cerr);
	    cerr << endl;
	    ppn(b, cerr);
	    cerr.precision(6);
	}
    
	// Check to see if the scattering is over.
    
	if (cpu_time_check < 0
	    && cpu_time() - cpu_save > abs(cpu_time_check)) return;
     
	if(b->get_time()>=t_out) {
	    cerr<< " terminate on t_out" << b->get_time()<<endl;
	    real ekin, epot;
	    calculate_energy(b, ekin, epot);
	    if(input.verbose==1) {
	
		cerr << "Time = " << b->get_time() 
		     << "  Etot = " << ekin + epot 
		     << "  Tinf = " << ekin/epot << endl;
	    }
	    t_out += dt_out;
	}

	if(b->get_time()>=t_end) {
	    cerr << "Early termination"<<endl;
	    experiment.set_scatter_discriptor(stopped);
	    experiment.set_stop(true);
	    real ekin, epot;
	    calculate_energy(b, ekin, epot);
	    cerr << "Time = " << b->get_time() 
		 << "  Etot = " << ekin + epot << endl;
	    break;
	}
      
	de_merge += merge_collisions(b);
	   
	make_tree(b, DYNAMICS, STABILITY, K_MAX, false);
	   
	int unbound = extend_or_end_scatter(b, ttf, false);
	if(unbound==2) {   // two body system is bound but we stop anyway
	    experiment.set_final_bound(true);
	    unbound = 1;   // but stop anyway
	}
	//bool unbound = tree_is_unbound(b, ttf, false);
	//make_tree(b, DYNAMICS, STABILITY, K_MAX, debug);
	if (unbound || terminate) break;
    
	//	if(experiment.get_form_changes()>1) 
	//	  cerr << "Resonant encounter: " << get_normal_form(b) << endl;
	strcpy(previous_form, current_form);
	strcpy(current_form, get_normal_form(b));
	if(strcmp(previous_form, current_form))
	    experiment.inc_form_changes();
     
	//	print_normal_form(b, cerr);
    }
   
    b->flatten_node();
    real etot_error = calculate_energy_from_scratch(b, kin, pot) 
	- etot_init - de_merge;
    experiment.set_energy_error(etot_error);
  
    make_tree(b, DYNAMICS, STABILITY, K_MAX, debug);
   
    // add final report
    experiment.final_scatter_exp(b);

}
#endif

#else

void slave_part_of_experiment(scatter_input input,
			      scatter_exp &experiment)
{
  cerr << "Random seed = " << get_initial_seed()
       << "  n_rand = " << get_n_rand() << flush << "\n";
    
  sdyn *b = mkscat(input.init_string);
  //  ppn(b, cerr);

  //        b->log_history(argc, argv);
  b->flatten_node();
    
  real kin, pot;
  real etot_init = calculate_energy_from_scratch(b, kin, pot);
  if(input.verbose==1) {
    cerr << "Time = " << 0 << "  Etot = " << kin + pot 
	 << " (" << kin << ", " << pot
	 << ")  Tinf = " << kin/pot << endl;
  }
  make_tree(b, !DYNAMICS, STABILITY, K_MAX, input.debug);
  ppn(b, cerr);
  
  // Initial output:
  cerr << "*** Initial configuration (random seed = "
       << get_initial_seed() << "):\n";
  print_normal_form(b, cerr);
  b->set_name("root");

    
  vec center = b->get_pos();
  print_structure_recursive(b, 0., center, true, true, 4);
    
  // Integrate the system to completion:
  scatter(b, input, experiment);

  // Final output:
  cerr.precision(6);
  if (input.debug) cerr << endl;

  cerr << "*** Final system configuration (time = " 
       << b->get_time() << "):\n";
  print_normal_form(b, cerr);
  b->set_name("root");
  ppn(b, cerr);
  //  vec center = b->get_pos();
  print_structure_recursive(b, 0., center, true, true, 4);
  
  cerr << "Nearest neighbor information." << endl;
  for(int i=0; i<experiment.get_n_initial(); i++) {
    cerr << " id= " << experiment.get_min_nn_of_label()[i] << " with id= " 
	 << experiment.get_min_nn_label()[i] << " at distance " 
	 << experiment.get_min_nn_dr2()[i] << endl;
  }

  cerr << "Energy error = " << experiment.get_energy_error() << endl;

  cerr << "Final Normal form:  ";
  print_normal_form(b, cerr);

  cerr << "\n\n"; 
  //
}

void master_part_of_experiment(scatter_input input,
			       scatter_exp &experiment)
{
  int random_seed = srandinter(input.seed, input.n_rand);
  
  // initialize scatter_hist
  sdyn *b = mkscat(input.init_string);
  scatter_hist *hi = initialize_scatter_hist(b);
  delete b;
  b = NULL;
  
  int n_exp = 0;
  real de = 0;
  for (n_exp = 0; n_exp < input.n_experiments; n_exp++) {
    
    if (input.n_experiments > 1) cerr << "Experiment #" << n_exp+1 
				      << ": " << "\n";
    slave_part_of_experiment(input, experiment);
    
    hi->add_scatter_hist(experiment, 0);
    cerr << "scatter history" << flush << endl;
    hi->put_scatter_hist(cerr, true);
    cerr << " N done = " << n_exp+1 << endl;
  }
}

void execute_scatter_experiment(scatter_input input)
{
    scatter_exp experiment; 

    int myid = 0;
    int master = 0;

    if (myid==master)
	master_part_of_experiment(input, experiment);
    else
	slave_part_of_experiment(input, experiment);
}

main(int argc, char **argv)
{

  scatter_input input;
  const char* default_init  
    = "-M 1 -rm 3 -v 1 -t -r1 0 -r2 0 -q 1 -p -a 1 -q 1 -r1 0 -r2 0";
    //iota-Ori initial conditions: "-M 0.879 -rm 3 -v 0.0071 -t -r1 0.0508 -r2 0.0348 -e 0 -q 0.567 -p -a 1 -q 1 -r1 0.0394 -r2 0.0394";        // Iota Ori Problem 
  
  // identical binary collision
  //const char* default_init  
  //    = "-M 1 -rm 3 -v 1 -t -r1 0 -r2 0 -q 1 -p -a 1 -q 1 -r1 0 -r2 0";

  strcpy(&input.init_string[0], default_init);

  bool D_flag = false;
  check_help();

  extern char *poptarg;
  int c;
  const char *param_string = "A:c:C:d:D:g:i:n:N:ps:t:v";

  while ((c = pgetopt(argc, argv, param_string,
		  "$Revision$", _SRC_)) != -1)
    switch(c) {

    case 'A': input.eta = atof(poptarg);
      break;
    case 'c': input.cpu_time_check = atof(poptarg);
      break;
    case 'C': input.snap_cube_size = atof(poptarg);
      break;
    case 'd': input.dt_out = atof(poptarg);
      break;
    case 'D': D_flag = TRUE;
      input.dt_snap = atof(poptarg);
      break;
    case 'g': input.tidal_tol_factor = atof(poptarg);
      break;
    case 'i': strcpy(&input.init_string[0], poptarg);
      break;
    case 'n': input.n_experiments = atoi(poptarg);
      break;   
    case 'N': input.n_rand = atoi(poptarg);
      break;
    case 'p': input.pipe = 1 - input.pipe;
      break;
    case 't': input.delta_t = atof(poptarg);
      break;
    case 'v': input.debug = 1 - input.debug;
              input.verbose = 1;
      break;
    case 's': input.seed = atoi(poptarg);
      break;
    case '?': params_to_usage(cerr, argv[0], param_string);
      get_help();
    }            

  cerr << input << endl;

  if (!D_flag) input.dt_snap = input.delta_t; // Guarantee output at end

  execute_scatter_experiment(input);

  //    cerr << "scatter history" << flush << endl;
  //    hi->put_scatter_hist(cerr, false);
  //    cerr << " N done = " << n_exp << endl;
}
#endif








