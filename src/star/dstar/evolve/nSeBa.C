//// SeBa:        Binary evolution program SeBa.
////              computes the evolution of a binary given any 
////              initial conditions (M, m, a, e).
////             
//// Output:      in the form of the following files:
////              -init.dat           contains selected initial conditons.
////              -SeBa.data           contains binary evolution histories
////              -binev.data          contains remnant formation information
////
////              in the form of standard output (cerr).
////
////              Initialized parameters include:
////              - mass of the most massice component (primary star)
////              - mass of its binary companion (secondary star)
////              - semi-major axis of the binary system
////              - orbital eccentricity
////
////              routines incuded can be found in double_star.h.
////              The mass function routines are adoped from mkmass.C
////              and are defined in starbase.h
//// 
////              externally visible routines are:
////              -get_random_mass_ratio
////              -get_random_semi_major_axis
////              -get_random_eccentricity
////              The two utilities for the various parameter are:
////              -extract_...._distribution_type_string(....)
////              and
////              -type_string(char*)
////
////              The executable takes initial conditions (see Options)
////              and returns randomized binary parameters.                 
////                 
//// Options:   -M    upper primary mass limit [100[Msun]]
////            -m    lower limit to primary mass [0.1[Msun]]
////            -x    mass function exponent in case of power law [-2.35]
////            -F/f  mass function option: 0) Equal mass
////                                        1) Power-law [default]
////                                        2) Miller & Scalo
////                                        3) Scalo
////                                        4) Kroupa
////            Option -F requires one of the following strings:
////                      (mf_Power_Law, Miller_Scalo, Scalo, Kroupa)
////                   -f requires the appropriate interger (see mkmass.C)
////             -A   maximum semi-major axis limit [1000000[Rsun]]   
////             -a   minimum semi-major axis limit [0] 
////             -y   exponent for a power-law distribution [0] (flat in log)
////             -G/g Semi major axis option: 0) Equal_sma
////                                          1) Power Law [default]
////                                          2) Duquennoy & Mayor (1987)
////            Option -G requires one of the following strings:
////                      (Equal_sma, sma_Power_Law, Duquennoy_Mayor)
////                   -g requires appropriate interger (see double_star.h)
////             -E   maximum eccentricity [1] 
////             -e   minimum eccentricity [0] 
////             -v   exponent for a power-law distribution 
////             -U/u eccentricity option: 0) Equal eccentricity
////                                       1) Power Law 
////                                       2) Thermal distribution [default]
////            Option -U requires one of the following strings:
////                      (Equal_ecc, ecc_Power_Law, Thermal_Distribution)
////                   -u requires appropriate interger (see double_star.h)
////             -Q   maximum mass ratio [1]
////             -q   minimum mass ratio [<-m option/selected primary mass>]
////             -w   exponent for a power-law distribution  
////             -P/p eccentricity option: 0) constant mass ratio
////                                       1) Flat_q
////                                       2) Power Law 
////                                       3) Hogeveen (1992)
////            Option -P requires one of the following strings:
////                      (Equal_q, Flat_q, qf_Power_Law, Hogeveen)
////                   -p requires appropriate interger (see double_star.h)
////
////            -I select input file for reading initial conditions.
////               -uses: double_star::dump as input format.  [no default]
////            -R select random initial conditions    [false]
////               with parameters as discribed above.   
////            -n number of binaries to be simulated.  [1]
////               Options: -I all binaries in input file are computed.
////                        -R the number of binaries indicated.
////                        oterwise one binary is simulated with
////                        -M, -m, -a, -e as initial conditions.
////
//   Note:  libnode.a is referenced for the routines which produce the 
//          mass function
//
//	version 1.0	Simon Portegies Zwart, Utrecht, 1992
//                      -First version with class structure
//	version 2.0	Simon Portegies Zwart, Utrecht, 1994
//                      -Coupling to starlab
//	version 3.0	Simon Portegies Zwart, Amsterdam, June 1997
//	version 3.3	Simon Portegies Zwart, Cambridge, March 1999
//

#include "node.h" 
#include "double_star.h"
#include "main_sequence.h"
//#include "dstar_to_dyn.h"

#ifdef TOOLBOX

local bool read_binary_params(ifstream& in, real &m_prim, 
			      real &m_sec, real &sma, real &ecc) {

  if(in.eof())
    return false;

  // reading from input file
  int id, time, tpp, tps; 
  real Rlp, Rls;

  in >>id >> time >> sma >> ecc 
     >> tpp >> m_prim >> Rlp >> tps >> m_sec >> Rls;

      PRC(m_prim);PRC(m_sec);PRC(sma);PRL(ecc);

  return true;
}

/*-----------------------------------------------------------------------------
 *  binev  --
 *-----------------------------------------------------------------------------
 */
local bool  evolve_binary(node * bi,
                          real start_time, real end_time,
			  bool stop_at_merger_or_disruption,
			  bool stop_at_remnant_formation) { 

  double_star* ds = dynamic_cast(double_star*, 
				 bi->get_starbase());
  
  //		Setup star from input data.
  real dt, time=start_time;
  ds->evolve_element(time);
  
  ds->dump("SeBa.data", true);

  if (!bi->is_root() &&
      bi->get_parent()->is_root()) 

    do {

      dt = ds->get_evolve_timestep() + cnsts.safety(minimum_timestep);
      time = min(time+dt, end_time);

      ds->evolve_element(time);

      if (stop_at_merger_or_disruption &&
	  (ds->get_bin_type() == Merged || 
	   ds->get_bin_type() == Disrupted))
	return false;

      if (stop_at_remnant_formation &&
	 (ds->get_primary()->remnant() || ds->get_secondary()->remnant()))
	return false;

    }
    while (time<end_time);

  ds->dump("SeBa.data", true);
    
  return true;

}

void main(int argc, char ** argv) {

    bool e_flag = false;
    bool R_flag = false;
    bool F_flag = false;
    bool I_flag = false;
    bool P_flag = false;
    bool U_flag = false;
    bool G_flag = false;

    bool stop_at_merger_or_disruption = false;
    bool stop_at_remnant_formation = false;
    bool random_initialization = false;
    stellar_type type = Main_Sequence;
    binary_type bin_type = Detached;
    real binary_fraction = 1.0;

    int n_init = 0;
    int n = -1;

    real  m_prim;
    real  m_sec;
    real  sma;
    real  ecc;

    real m_tot = 1;
    real r_hm = 1;
    real t_hc = 1;

    char *mfc = new char[64];
    mass_function mf = mf_Power_Law;
    real m_min = 0.1;
    real m_max = 100;
    real m_exp = -2.35;
    char *qfc = new char[64];
    mass_ratio_distribution qf = Flat_q;
    real q_min = 0;
    real q_max = 1;
    real q_exp = 0;
    char *afc = new char[64];
    sma_distribution af = sma_Power_Law;
    real a_min = 0;
    real a_max = 1.e+6; 
    real a_exp = -1;                           
    char *efc = new char[64];
    ecc_distribution ef = Thermal_Distribution;
    real e_min = -1;    // allow detection of constant eccentricity
    real e_max = 1;
    real e_exp;

    real start_time = 0;
    real end_time   = 35;

    char* input_filename;

    int input_seed=0; 
    char seedlog[64];
    char paramlog[65];

    check_help();

    extern char *poptarg;
    int c;
    const char *param_string = "n:N:RDSM:m:x:F:f:A:a:y:G:g:E:e:v:U:u:Q:q:T:I:w:P:p:n:s:";

    while ((c = pgetopt(argc, argv, param_string,
		    "$Revision$", _SRC_)) != -1)
	switch(c) {
            case 'R': random_initialization = true;
		      break;
            case 'D': stop_at_merger_or_disruption = true;
		      break;
            case 'S': stop_at_remnant_formation = true;
		      break;
            case 'M': m_max = atof(poptarg);
		      break;
            case 'm': m_min = atof(poptarg);
		      break;
            case 'x': m_exp = atof(poptarg);
		      break;
	    case 'F': F_flag = true;
		      strcpy(mfc, poptarg);
	              break;
	    case 'f': mf = (mass_function)atoi(poptarg);
	              break;
            case 'A': a_max = atof(poptarg);
		      break;
            case 'a': a_min = atof(poptarg);
		      break;
            case 'y': a_exp = atof(poptarg);
		      break;
	    case 'G': G_flag = true;
		      strcpy(afc, poptarg);
	              break;
	    case 'g': af = (sma_distribution)atoi(poptarg);
	              break;
            case 'E': e_max = atof(poptarg);
		      break;
            case 'e': e_min = atof(poptarg);
		      break;
            case 'v': e_exp = atof(poptarg);
		      break;
	    case 'U': U_flag = true;
		      strcpy(efc, poptarg);
	              break;
	    case 'u': ef = (ecc_distribution)atoi(poptarg);
	              break;
            case 'Q': q_max = atof(poptarg);
		      break;
            case 'q': q_min = atof(poptarg);
		      break;
            case 'T': end_time = atof(poptarg);
		      break;
            case 'I': I_flag = true;
		      strcpy(input_filename, poptarg);
		      break;
            case 'w': q_exp = atof(poptarg);
		      break;
	    case 'P': P_flag = true;
		      strcpy(qfc, poptarg);
	              break;
	    case 'p': qf = (mass_ratio_distribution)atoi(poptarg);
	              break;
	    case 'n': n = atoi(poptarg);
	              break;
	    case 'N': n_init = atoi(poptarg);
	              break;
	    case 's': input_seed = atoi(poptarg);
		      break;
            case '?': params_to_usage(cerr, argv[0], param_string);
		      exit(1);
	}

    int actual_seed = srandinter(input_seed);
    cerr << "random number generator seed = " << actual_seed << endl;
    sprintf(paramlog, 
   "         alpha  = %3.1f\n         lambda = %3.1f\n         beta   = %3.1f",
	    cnsts.parameters(common_envelope_efficiency),
	    cnsts.parameters(envelope_binding_energy),
	    cnsts.parameters(specific_angular_momentum_loss));


    if (n <= 0) {
      hdyn *b;

      b = get_hdyn();
      if (c_flag == TRUE)
        b->log_comment(comment);
      b->log_history(argc, argv);
      b->get_starbase()->set_stellar_evolution_scaling(m_tot, r_hm, t_hc);

      addstar(root, start_time, type);

      pp(root, cerr);
      cerr << endl;

      ds = new_double_star(the_binary, sma, ecc, start_time, 
			   i + n_init, bin_type);

    put_node(b);

    real t_out, dt = t_end/n;
    for (int i=0; i<n; i++) {
        t_out = dt * (i+1);
      
        for_all_daughters(node, b, bi) 
	    bi->get_starbase()->evolve_element(t_out);

        put_node(b);
    }


    }
    else {

    if(F_flag)
	mf = extract_mass_function_type_string(mfc);
    delete mfc;
    if(G_flag)
	af = extract_semimajor_axis_distribution_type_string(afc);
    delete afc;
    if(U_flag)
	ef = extract_eccentricity_distribution_type_string(efc);
    delete efc;
    if(P_flag)
	qf = extract_mass_ratio_distribution_type_string(qfc);
    delete qfc;

    actual_seed = srandinter(input_seed);
    sprintf(seedlog, "       random number generator seed = %d",actual_seed);

    ifstream infile(input_filename);
    if(I_flag) {
      if (!infile) cerr << "error: couldn't create file "
	                << input_filename <<endl;
	cerr << "Reading input from file "<< input_filename <<endl;
    }

    node *root, *the_binary;
    double_star *ds;

    for (int i=0; i<n; i++) {

      if(I_flag) {

	if(read_binary_params(infile, m_prim, m_sec, sma, ecc)) 
	  n=i+2;
	else
	  break;

      }
      else if (random_initialization) 
	mkrandom_binary(m_min, m_max, mf, m_exp,
			q_min, q_max, qf, q_exp,
			a_min, a_max, af, a_exp,
			e_min, e_max, ef, e_exp,
			m_prim, m_sec, sma, ecc);
      else {
	m_prim = m_max;
	m_sec  = m_min;
	sma    = a_min;
	ecc    = e_min;
	n = 1;
      }

      //      PRC(m_prim);PRC(m_sec/m_prim);PRC(sma);PRL(ecc);

      // Create flat tree 
      root = mknode(1);

      root->log_history(argc, argv);
      root->log_comment(seedlog);
      root->log_comment(paramlog);
      root->print_log_story(cout);
      root->set_mass(1);
      root->get_starbase()->set_stellar_evolution_scaling(m_prim, r_hm, t_hc);
      
      the_binary = root->get_oldest_daughter();
      add_secondary(the_binary, m_sec/m_prim);
      
      addstar(root, start_time, type);

      pp(root, cerr);
      cerr << endl;

      ds = new_double_star(the_binary, sma, ecc, start_time, 
			   i + n_init, bin_type);

      //      PRL(((star*)the_binary->get_oldest_daughter()->get_starbase())->get_companion());

      //      PRL(the_binary);
      //      PRL(ds);

      ds->dump(cerr, false);
      ds->set_use_hdyn(false);
      ds->get_primary()->set_identity(0);
      ds->get_secondary()->set_identity(1);

      //      node *b      = root->get_oldest_daughter();
      //      starbase *s  = b->get_starbase();
      //      star *st     = dynamic_cast(star*, b->get_starbase());
   
      evolve_binary(the_binary, start_time, end_time, 
		    stop_at_merger_or_disruption, stop_at_remnant_formation);

      the_binary->get_starbase()->dump(cerr, false);

      // put_node(root);

      rmtree(root);
      //      delete ds->get_primary();
      //      delete ds->get_secondary();
      //      delete ds;
      //      delete root->get_oldest_daughter()->get_younger_sister();
	
    }
  }
}

#endif
