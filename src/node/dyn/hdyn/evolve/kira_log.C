
       //=======================================================//    _\|/_
      //  __  _____           ___                    ___       //      /|\ ~
     //  /      |      ^     |   \  |         ^     |   \     //          _\|/_
    //   \__    |     / \    |___/  |        / \    |___/    //            /|\ ~
   //       \   |    /___\   |  \   |       /___\   |   \   // _\|/_
  //     ___/   |   /     \  |   \  |____  /     \  |___/  //   /|\ ~
 //                                                       //            _\|/_
//=======================================================//              /|\ ~

// Functions associated with log output, snapshots, etc.
//
// Externally visible functions:
//
//	void print_dstar_params
//	bool print_dstar_stats
//	void get_energies_with_external
//	void print_statistics
//	void update_cpu_counters
//	void log_output
//	void snap_output

#include "hdyn.h"
#include "star/dstar_to_kira.h"

typedef  struct {
    real  dt;
    real  dt_a;
    hdyn*  b;
} dt_pair, *dt_pair_ptr;

local int compare_dt(const void * pi, const void * pj)	  // increasing dt
{
    if (((dt_pair_ptr) pi)->dt < ((dt_pair_ptr) pj)->dt)
        return -1;
    else if (((dt_pair_ptr)pi)->dt > ((dt_pair_ptr)pj)->dt)
        return +1;
    else
        return 0;
}

typedef  struct {
    real  count;
    hdyn*  b;
} count_pair, *count_pair_ptr;

local int compare_steps(const void * pi, const void * pj) // decreasing count
{
    if (((count_pair_ptr) pi)->count > ((count_pair_ptr) pj)->count)
        return -1;
    else if (((count_pair_ptr)pi)->count < ((count_pair_ptr)pj)->count)
        return +1;
    else
        return 0;
}

int get_effective_block(real dt)
{
    // For general timestep dt, find the effective block it is in, by
    // determining the smallest k for which the step can be written as
    //
    //		dt = n * 2^{-k}
    //
    // for integral n.

    int k = 0;
    real dtb = 1;

    while (fmod(dt, dtb) != 0 && k < 50) {
	k++;
	dtb /= 2;
    }

    return k;
}

static bool allow_block_check = true;
void set_block_check(bool b)		// default = true
{
    allow_block_check = b;
}

#define NTOP 10
#define NBOT 10
#define N_MISMATCH 5

local void print_timestep_stats(hdyn* b, int max_count = N_MISMATCH)
{
    // Time step statistics.

    real dt_mean = 0, dt_min = VERY_LARGE_NUMBER, dt_max = -dt_min;

    real n_dt = 0;		// actually these are integer counters
    real n_dt_p = 0;
    real n_dt_u = 0;
    real total_steps = 0;
    real total_steps_a = 0;
    real total_steps_p = 0;

    // Timestep histograms:

    int count[30],		// all nodes, perturbed steps
        count_a[30],		// all nodes, actual steps
        count_p[30],		// all perturbed nodes
        count_u[30],		// all unperturbed nodes, unperturbed steps
        count_ue[30],		// all unperturbed nodes, effective blocks
        count_next[30];		// next_time distribution

    int i;
    for (i = 0; i < 30; i++)
	count[i] = count_a[i]
	    	 = count_p[i]
		 = count_u[i]
		 = count_ue[i]
		 = count_next[i]
		 = 0;

    // Count the total number of particles/nodes.

    int n = 0;
    for_all_nodes(hdyn, b, bb)  n++;
    n--;

    dt_pair_ptr list_dt  = new dt_pair[n];
    dt_pair_ptr list_dtb = new dt_pair[n];

    // Total timestep counters:

    count_pair_ptr steps_inst  = new count_pair[n];
    count_pair_ptr steps_delta = new count_pair[n];
    count_pair_ptr f_dir_delta = new count_pair[n];
    count_pair_ptr f_ind_delta = new count_pair[n];

    n = 0;
    int np = 0, nd = 0, ni = 0;

    real delta_steps_tot = 0;
    real delta_f_dir_tot = 0;
    real delta_f_ind_tot = 0;

    bool print_dtb = false;
    int count_mis = 0;

    for_all_nodes(hdyn, b, bb)
	if (bb != b) {

	    real dt = bb->get_timestep();

	    // This dt is the timestep of a top-level or perturbed binary
	    // node, and the perturbed timestep of an unperturbed binary.

	    // Actual timestep:

	    real dt_a = dt;
	    if (bb->get_kepler())
		dt_a = bb->get_unperturbed_timestep();

	    // Don't include younger binary sisters in *any* statistic.

	    if (bb->is_top_level_node() || bb->get_elder_sister() == NULL) {

		// Basic stats (all steps):

		dt_mean += dt_a;
		dt_min = Starlab::min(dt_min, dt_a);
		dt_max = Starlab::max(dt_max, dt_a);

		n_dt += 1;
		real steps = rint(1/dt);
		real steps_a = rint(1/dt_a);
		total_steps += steps;
		total_steps_a += steps_a;

		// Update perturbed_steps counters.

		if (!bb->get_kepler()) {

		    list_dt[np].dt = dt;
		    list_dt[np].dt_a = dt;
		    list_dt[np].b = bb;
		    steps_inst[np].count = steps;
		    steps_inst[np].b = bb;
		    np++;
		}

		// 1. All perturbed steps, including unperturbed
		//    binaries: count.

		i = 0;
		real dtbin = 1;
		while (dt < dtbin) {
		    i++;
		    dtbin *= 0.5;
		}
		if (i > 29) i = 29;
		count[i]++;

		if (!bb->get_kepler()) {

		    // 2. Perturbed nodes only: count_p.

		    total_steps_p += steps;
		    n_dt_p += 1;
		    count_p[i]++;

		}

		// 3. All actual steps, including unperturbed
		//    binaries: count_a.

		i = 0;
		dtbin = 1;
		while (dt_a < dtbin) {
		    i++;
		    dtbin *= 0.5;
		}
		if (i > 29) i = 29;
		count_a[i]++;

		if (bb->get_kepler()) {

		    // 4a. Unperturbed nodes only, unperturbed steps: count_u.

		    n_dt_u += 1;
		    count_u[i]++;

		    // 4b. Unperturbed nodes only, effective blocks: count_ue.

		    int kb = get_effective_block(dt_a);
		    if (kb > 29) kb = 29;
		    count_ue[kb]++;
		}

		// 5. Next_time, all nodes, actual steps: count_next.

		real dt_next = bb->get_next_time() - bb->get_system_time();
		int kb = get_effective_block(dt_next);
		real dtb = pow(2.0, -kb);

		list_dtb[n].dt = -kb;
		list_dtb[n].dt_a = dt_a;
		list_dtb[n].b = bb;

		if (allow_block_check && (bb->get_kepler() || dt_a != dtb)) {

		    if (++count_mis <= max_count) {

		        if (!print_dtb) {
			    cerr << endl << "  Actual and block timesteps:"
				 << endl;
			    print_dtb = true;
			}

			if (bb->get_kepler())
			  cerr << "    unperturbed ";
			else
			  cerr << "    ***mismatch ";

			cerr << bb->format_label() << ":  ";
			PRC(kb), PRC(dt_a); PRL(dtb);
			
		    }
		}

		if (kb > 29) kb = 29;
		count_next[kb]++;		// next_time

		// Total timestep/force counters:

		// Instantaneous:

		steps_delta[n].count = bb->get_steps();
		if (find_qmatch(bb->get_dyn_story(), "prev_steps"))
		    steps_delta[n].count -= getrq(bb->get_dyn_story(),
						  "prev_steps");
		putrq(bb->get_dyn_story(), "prev_steps", bb->get_steps());
		steps_delta[n].b = bb;
		delta_steps_tot += steps_delta[n].count;

		n++;

		// Direct (top-level/GRAPE):

		f_dir_delta[nd].count = bb->get_direct_force();
		if (find_qmatch(bb->get_dyn_story(), "prev_f_dir"))
		    f_dir_delta[nd].count -= getrq(bb->get_dyn_story(),
						   "prev_f_dir");
		putrq(bb->get_dyn_story(), "prev_f_dir",
		      bb->get_direct_force());
		f_dir_delta[nd].b = bb;
		delta_f_dir_tot += f_dir_delta[nd].count;
		nd++;

		// Indirect (low-level/front-end):

		f_ind_delta[ni].count = bb->get_indirect_force();
		if (find_qmatch(bb->get_dyn_story(), "prev_f_ind"))
		    f_ind_delta[ni].count -= getrq(bb->get_dyn_story(),
						   "prev_f_ind");
		putrq(bb->get_dyn_story(), "prev_f_ind",
		      bb->get_indirect_force());
		f_ind_delta[ni].b = bb;
		delta_f_ind_tot += f_ind_delta[ni].count;
		ni++;
	    }
	}

    if (count_mis > max_count) {
	if (max_count > 0)
	    cerr << "  .\n  .\n  (plus "
		 << count_mis-max_count
		 << " more mismatches)" << endl;
	else
	    cerr << "  *** " << count_mis << " timestep mismatches" << endl;
    }

    //
    //----------------------------------------------------------------------
    //
    // Print out the results:

    // First print the timestep histograms...

    cerr << endl << "  Timesteps (younger binary components excluded,"
	 << endl << "             mean = " << dt_mean/Starlab::max((int)rint(n_dt), 1)
	 << "  min = " << dt_min << "  max = " << dt_max << "):"
	 << endl;

    if (n_dt_u > 0)
	cerr << endl << "  all nodes (perturbed steps, bin #1 = 1, dt /= 2):"
	     << endl << "        ";
    else
	cerr << endl << "  all nodes (actual steps, bin #1 = 1, dt /= 2):"
	     << endl << "        ";

    for (i =  0; i < 10; i++) fprintf(stderr, " %5d", count[i]);
    cerr << endl << "        ";
    for (i = 10; i < 20; i++) fprintf(stderr, " %5d", count[i]);
    cerr << endl << "        ";
    for (i = 20; i < 30; i++) fprintf(stderr, " %5d", count[i]);

    cerr << endl << "  total = " << n_dt
	 << ",  weighted timestep sum = "
	 << total_steps/n_dt << endl;

    if (n_dt_u > 0) {

	cerr << endl << "  all nodes (actual steps):"
	     << endl << "        ";
	for (i =  0; i < 10; i++) fprintf(stderr, " %5d", count_a[i]);
	cerr << endl << "        ";
	for (i = 10; i < 20; i++) fprintf(stderr, " %5d", count_a[i]);
	cerr << endl << "        ";
	for (i = 20; i < 30; i++) fprintf(stderr, " %5d", count_a[i]);

	cerr << endl << "  total = " << n_dt
	     << ",  weighted timestep sum = "
	     << total_steps_a/n_dt << endl;

	if (n_dt_p > 0) {
	    cerr << endl << "  excluding unperturbed systems:"
		 << endl << "        ";
	    for (i =  0; i < 10; i++) fprintf(stderr, " %5d", count_p[i]);
	    cerr << endl << "        ";
	    for (i = 10; i < 20; i++) fprintf(stderr, " %5d", count_p[i]);
	    cerr << endl << "        ";
	    for (i = 20; i < 30; i++) fprintf(stderr, " %5d", count_p[i]);

	    cerr << endl << "  total = " << n_dt_p
		 << ",  weighted timestep sum = "
		 << total_steps_p/n_dt_p
		 << endl;
	}

	cerr << endl << "  unperturbed systems only:"
	     << endl << "        ";
	for (i =  0; i < 10; i++) fprintf(stderr, " %5d", count_u[i]);
	cerr << endl << "        ";
	for (i = 10; i < 20; i++) fprintf(stderr, " %5d", count_u[i]);
	cerr << endl << "        ";
	for (i = 20; i < 30; i++) fprintf(stderr, " %5d", count_u[i]);

	cerr << endl << "  total = " << n_dt_u << endl;

	cerr << endl << "  unperturbed systems only (effective blocks):"
	     << endl << "        ";
	for (i =  0; i < 10; i++) fprintf(stderr, " %5d", count_ue[i]);
	cerr << endl << "        ";
	for (i = 10; i < 20; i++) fprintf(stderr, " %5d", count_ue[i]);
	cerr << endl << "        ";
	for (i = 20; i < 30; i++) fprintf(stderr, " %5d", count_ue[i]);
	cerr << endl;
    }

    cerr << endl
	 << "  next_step distribution (all nodes):"
	 << endl << "        ";
    for (i =  0; i < 10; i++) fprintf(stderr, " %5d", count_next[i]);
    cerr << endl << "        ";
    for (i = 10; i < 20; i++) fprintf(stderr, " %5d", count_next[i]);
    cerr << endl << "        ";
    for (i = 20; i < 30; i++) fprintf(stderr, " %5d", count_next[i]);
    cerr << endl;

    // ...then the list of shortest steps and force counters.

    if (n > NBOT+NTOP) {

        int nb = NBOT;
	if (nb > np) nb = np;

	// Print out nb shortest perturbed time steps.

	cerr << endl << "  " << nb << " shortest perturbed time steps:"
	     << endl << endl;

	qsort((void *)list_dt, (size_t)np, sizeof(dt_pair), compare_dt);

	for (i = 0; i < nb; i++)
	    if (list_dt[i].b)
	        fprintf(stderr, "    %3d.     %-15s  %.4e\n",
			i+1,
			list_dt[i].b->format_label(),
			list_dt[i].dt);
	    else
	        fprintf(stderr, "    %3d.     NULL\n", i+1);

	// Print out NBOT shortest effective block steps.

	cerr << endl << "  " << NBOT << " shortest effective block steps:"
	     << endl << endl;

	qsort((void *)list_dtb, (size_t)n, sizeof(dt_pair), compare_dt);

	for (i = 0; i < NBOT; i++)
	    fprintf(stderr, "    %3d.     %-15s  %3d   %.4e   %.4e\n",
		    i+1,
		    list_dtb[i].b->format_label(),
		    -(int)list_dtb[i].dt,	   // "dt" here is really -kb
		    pow(2.0, list_dtb[i].dt),
		    list_dtb[i].dt_a);

	// Print out top NTOP particles in (1) instantaneous cost (steps),
	//				   (2) integrated cost since the
	//				       last output.
	//				   (3) direct ("GRAPE") forces
	//				       since the last output
	//				   (4) indirect ("front-end") forces
	//				       since the last output.

	// Sort the arrays in order of decreasing count:

	qsort((void *)steps_inst,  (size_t)np, sizeof(count_pair),
	      compare_steps);
	qsort((void *)steps_delta, (size_t)n,  sizeof(count_pair),
	      compare_steps);
	qsort((void *)f_dir_delta, (size_t)nd, sizeof(count_pair),
	      compare_steps);
	qsort((void *)f_ind_delta, (size_t)ni, sizeof(count_pair),
	      compare_steps);

	cerr << endl << "  Top " << NTOP
	     << " nodes by instantaneous (excl. unperturbed)"
	     << " and delta (all) steps:"
	     << endl << endl;

	fprintf(stderr, "             %-26s      %-30s\n",
		"--- instantaneous ---",
		"------- delta -------");
	fprintf(stderr, "            %-26s      %-30s\n\n",
		"(normalized to dt = 1) ",
		"(since last log output)");

	char tmp[1024];

	for (i = 0; i < NTOP; i++) {

	    if (i < np)
		sprintf(tmp, "%3d.     %-10s %10.0f",
			i+1,
			steps_inst[i].b->format_label(),
			steps_inst[i].count);
	    else
		strcpy(tmp, " ");
	    fprintf(stderr, "    %-35s", tmp);

	    if (i < n)
		sprintf(tmp, "%-10s %10.0f",
			steps_delta[i].b->format_label(),
			steps_delta[i].count);
	    else
		strcpy(tmp, " ");
	    fprintf(stderr, "      %-30s\n", tmp);
	}
	cerr << endl << "  total delta_steps = " << delta_steps_tot << endl;

	cerr << endl << "  Top " << NTOP
	     << " nodes by delta(force calculations):"
	     << endl << endl;

	fprintf(stderr, "             %-26s      %-30s\n",
		"------- direct ------",
		"------ indirect -----");

	for (i = 0; i < NTOP; i++) {

	    if (i < nd)
		sprintf(tmp, "%3d.     %-10s %10.0f",
			i+1,
			f_dir_delta[i].b->format_label(),
			f_dir_delta[i].count);
	    else
		strcpy(tmp, " ");
	    fprintf(stderr, "    %-35s", tmp);

	    if (i < ni && f_ind_delta[i].count > 0)
		sprintf(tmp, "%-10s %10.0f",
			f_ind_delta[i].b->format_label(),
			f_ind_delta[i].count);
	    else
		strcpy(tmp, " ");
	    fprintf(stderr, "      %-30s\n", tmp);
	}
	cerr << endl << "  total delta_f_dir = " << delta_f_dir_tot
	     << "  delta_f_ind = " << delta_f_dir_tot
	     << endl;
    }

    delete [] list_dt;
    delete [] list_dtb;
    delete [] steps_inst;
    delete [] steps_delta;
    delete [] f_dir_delta;
    delete [] f_ind_delta;
}

void print_dstar_params(dyn* b)			// b is binary center of mass
{
    if (((hdyn*)b)->get_use_dstar())
	print_binary_dstars(b);
}

bool print_dstar_stats(dyn* b, bool mass_function,    // b is binary
		       vec center, bool verbose)   // center of mass
{
    if (((hdyn*)b)->get_use_dstar()) {
	dstar_stats(b, mass_function, center, verbose);

	return true;
    }

    return false;
}

void print_statistics(hdyn* b,
		      int long_binary_output)		// default = 2
{
    // General system statistics (omit time output
    // and O(N^2) operations):

#if 1
    sys_stats(b,
	      0.5,			// energy cutoff
	      2,			// verbose output (int now)
	      (long_binary_output > 0),	// print binaries
	      (long_binary_output > 1),	// full binary_output
	      2,			// which lagr
	      false,			// don't print time
	      false,			// don't compute energy
	      false,			// don't allow n^2 ops
	      kira_calculate_energies,	// energy calculation function
	      print_dstar_params,	// allow access to dstar_to_kira
	      print_dstar_stats);	//     from dyn sys_stats function
#else
    cerr << "(sys_stats omitted)" << endl;
#endif

    // Statistics on time steps and bottlenecks:

#if 1
    print_timestep_stats(b);
    print_sort_counters();
#else
    cerr << "(statistics omitted)" << endl;
#endif
}

// Must set cod_pot before using these functions if the tidal
// potential is desired.  See log_output() below.

static real cod_pot = 0;
local void set_cod_pot(hdyn *b, vec cod_pos)
{
    cod_pot = 0;
    if (b->get_external_field()) {
	vec tmp;
	get_external_acc(b, cod_pos, tmp, cod_pot, tmp, tmp, true);
    }
}

local int n_bound(hdyn* b, vec& cod_vel)
{
    int nb = 0;
    for_all_daughters(hdyn, b, bb)
	if (0.5*square(bb->get_vel()-cod_vel) + bb->get_pot() - cod_pot < 0)
	    nb += bb->n_leaves();
    return nb;
}

local real m_bound(hdyn* b, vec& cod_vel)
{
    real mb = 0;
    for_all_daughters(hdyn, b, bb)
	if (0.5*square(bb->get_vel()-cod_vel) + bb->get_pot() - cod_pot < 0)
	    mb += bb->get_mass();
    return mb;
}

local void get_n_and_m_bound(hdyn* b, vec& cod_vel,
			     int& nb, real& mb)
{
    nb = 0;
    mb = 0;
    for_all_daughters(hdyn, b, bb)
	if (0.5*square(bb->get_vel()-cod_vel) + bb->get_pot() - cod_pot < 0) {
	    nb += bb->n_leaves();
	    mb += bb->get_mass();
    }
}



void update_cpu_counters(hdyn * b)
{
    real last_cpu = b->get_kira_counters()->cpu_time;
    real cpu = cpu_time() + b->get_thread_cpu();
    b->get_kira_counters()->cpu_time = cpu;
    b->get_kira_counters()->total_cpu_time += (cpu - last_cpu);
    write_counters_to_log(b);
}

// Local flag, possibly reset at initialization.

static bool with_density = true;
void force_nodensity() {with_density = false;}

void log_output(hdyn * b,
		real count, real steps,
		real count_top_level, real steps_top_level,
		kira_counters* kc_prev,
		real delta_t,
		int long_binary_output)		// default = 2
{
    // Standard log output (to stderr, note).

    int p = cerr.precision(STD_PRECISION);

    vec cod_pos, com_vel, cod_vel;
    compute_com(b, cod_pos, com_vel);

    cod_pos -= b->get_pos();		// CM quantities include the
    com_vel -= b->get_vel();		// root node pos and vel

    cod_vel = com_vel;

#if 1

    // Suppressed while debugging other parts of
    // the GRAPE-6 interface (Steve, 7/00).
    // Superceded with the with_density flag (Steve, 2/05).

    real cpu = cpu_time();
    cerr << endl << "computing densities..." << flush;

    // Possible that the density calculation will fail, in which case
    // we delete the "density_time" entry from the root dyn story.

    if (!with_density ||
	!kira_calculate_densities(b, cod_pos, cod_vel)) { // does nothing unless
	cerr << "(densities unavailable)" << endl;	  // GRAPE is present
	rmq(b->get_dyn_story(), "density_time");
    }

    cpu = cpu_time() - cpu;
    if (cpu < 0) cpu = 0;
    PRL(cpu);

#endif

    int n_bound;
    real m_bound;
    set_cod_pot(b, cod_pos);
    get_n_and_m_bound(b, cod_vel, n_bound, m_bound);

    cerr << endl;
    for (int k = 0 ; k < 40; k++) cerr << '-';

    // Adjust the time precision to resolve delta_t.

    real tt = b->get_system_time();
    int it = 1;
    if (tt > 0) it = (int)log10(tt) + 1;
    if (delta_t < 1) it -= (int)log10(delta_t)-4;    // resolve ~4 digits of dt
    if (it < STD_PRECISION) it = STD_PRECISION;

    int p2 = cerr.precision(it);
    cerr << endl << endl
	 << "Time = " << b->get_system_time();
    cerr.precision(p2);

    cerr << "  N = "    << b->n_leaves() << " (" << n_bound << " bound)"
	 << "  mass = " << total_mass(b) << " (" << m_bound << " bound)"
	 << endl;

    // Resort to C to get the formatting right!

//     int p = cerr.precision(2)
//     cerr << "          block steps = " << count
// 	 << "  total steps = " << steps
// 	 << "  steps/block = " << steps/Starlab::max(1.0, count)
// 	 << endl
// 	 << "          top-level:    " << count_top_level
// 	 << "                " << steps_top_level
// 	 << "                "
//	 << steps_top_level/Starlab::max(1.0, count_top_level)
// 	 << endl;
//     cerr.precision(p);

    fprintf(stderr, "          block steps = %8.2e", count);
    fprintf(stderr, "  total steps = %8.2e", steps);
    fprintf(stderr, "  steps/block = %6.1f\n", steps/Starlab::max(1.0, count));
    fprintf(stderr, "          top-level:    %8.2e", count_top_level);
    fprintf(stderr, "                %8.2e", steps_top_level);
    fprintf(stderr, "                %6.1f\n",
	    steps_top_level/Starlab::max(1.0, count_top_level));

    if (b->get_use_sstar())
	print_sstar_time_scales(b);

    cerr << "          root pos = " << b->get_pos() << endl
	 << "               vel = " << b->get_vel() << endl;
    cerr << endl;

    // Do energy output here using known data; sys_stats can only compute
    // energies using the front end, so suppress that option below.

    print_recalculated_energies(b, true, true);

    // Note: on return from print_recalculated_energies, hdyn::pot
    // is properly set, and includes the external field, if any.
    // The earlier "bound" quantities may be suspect...

    // Determine "internal" quantities and determine r_vir, kT, q, etc.
    // Use the story as a handy way to get the energies back without
    // adding more arguments.  Data saved at HIGH_PRECISION.

    real true_pot_int = getrq(b->get_dyn_story(), "potential_energy");
    real true_kin_int = getrq(b->get_dyn_story(), "kinetic_energy");

    // ***However*** for purposes of computing the virial radius and kT,
    // we want the top-level energies, not the totals, which include the
    // energies of internal binaries.  Compute the top-level quantitles
    // here using existing data.

    real total_mass = 0, top_pot_int = 0, top_kin_int = 0;
    int ntop = 0;
    for_all_daughters(hdyn, b, bb) {
	ntop++;
	total_mass += bb->get_mass();
	top_pot_int += bb->get_mass()*bb->get_pot();
	top_kin_int += bb->get_mass()*square(bb->get_vel()-b->get_vel());
    }

    // Extra output on external fields and center of mass energy:

    unsigned int ext = b->get_external_field();
    if (ext) {
	real pot_ext = get_external_pot(b);
	true_pot_int -= pot_ext;
	top_pot_int -= pot_ext;
	PRI(10);
	cerr << "external potential = " << pot_ext << " (";
	print_external(ext);
	cerr << ")" << endl;
    }

    // Top-level pot and kin were counted twice above.

    top_pot_int /= 2;
    top_kin_int /= 2;

    real kin_CM = 0.5*b->get_mass()*square(com_vel);
    PRI(10);
    cerr << "CM kinetic energy = " << kin_CM << endl;
    top_kin_int -= kin_CM;

    real r_virial = -0.5*square(total_mass)/top_pot_int;  // standard definition
    real virial_ratio = -top_kin_int			  // inefficient
	        / (top_pot_int + get_external_virial(b)); //   (recomputes com)

    PRI(10);
    cerr << "top-level internal potential = " << top_pot_int
	 << ",  kinetic = " << top_kin_int << endl;
    PRI(10); PRC(virial_ratio); PRL(r_virial); 

    // PRL(get_external_virial(b));

#if 0

    // Check some basics...

    vec cmvel = 0;
    real mm = 0;
    real k = 0, kk = 0;
    for_all_daughters(hdyn, b, bb) {
	mm += bb->get_mass();
	cmvel += bb->get_mass()*bb->get_vel();
	k += 0.5*bb->get_mass()*square(bb->get_vel());
	kk += 0.5*bb->get_mass()*square(bb->get_vel()-com_vel);
    }
    cmvel /= mm;
    real kcm = 0.5*mm*square(cmvel);
    PRC(mm); PRL(b->get_mass());
    PRL(cmvel); PRL(com_vel);
    PRC(kcm); PRL(kcm-kin_CM);
    PRC(k); PRC(kk); PRL(k - kk - kcm);
    PRL(-kk/(pot_int + get_external_virial(b)));
#endif

    cerr.precision(p);

    // Update, but don't print, counters if steps = 0.  Note that kc_prev
    // is set equal to kc in print_counters.  (Steve, 5/02)

    update_cpu_counters(b);
    print_counters(b->get_kira_counters(), (steps > 0), kc_prev);

    print_statistics(b, long_binary_output);
    cerr << flush;
}

void snap_output(hdyn * b, real steps, int& snaps,
		 bool reg_snap, bool last_snap,
		 const char * snap_save_file,
		 xreal t, xreal ttmp, real t_end,
		 real& t_snap, real dt_snap, bool verbose)
{
    // Save a snapshot.

    update_cpu_counters(b);
    b->set_time(t);

    if (last_snap) {

	// Write (overwrite) snap_save_file.

	ofstream file(snap_save_file);

	if (file) {
	    set_complete_system_dump(true);
	    put_node(b, file, b->get_kira_options()->print_xreal);
	    set_complete_system_dump(false);
	    file.close();
	    cerr << "Snapshot saved in file "
		 << snap_save_file
		 << " (save CPU = "
		 << cpu_time() - b->get_kira_counters()->cpu_time
		 << ")\n";
	} else {
	    if (snap_save_file == NULL)
		cerr << "No save file specified.\n";
	    else
		cerr << "Error opening save file "
		     << snap_save_file << "\n";
	}
    }

    if (reg_snap) {

	if (verbose) cerr << endl;
	    cerr << "Snapshot output #" << ++snaps
		 << " at steps = " << steps
		 << ", time = " << b->get_system_time() << endl;
	if (verbose) cerr << endl;

	set_complete_system_dump(true);
	put_node(b, cout, b->get_kira_options()->print_xreal);
	set_complete_system_dump(false);
	cout << flush;

	// if (ttmp > t_end) exit(0);	// do this in kira, not here...
    }
}
