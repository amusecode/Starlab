
       //=======================================================//    _\|/_
      //  __  _____           ___                    ___       //      /|\
     //  /      |      ^     |   \  |         ^     |   \     //          _\|/_
    //   \__    |     / \    |___/  |        / \    |___/    //            /|\
   //       \   |    /___\   |  \   |       /___\   |   \   // _\|/_
  //     ___/   |   /     \  |   \  |____  /     \  |___/  //   /|\     
 //                                                       //            _\|/_
//=======================================================//              /|\

// Functions associated with escapers.
//
// Externally visible functions:
//
//	void check_and_remove_escapers

#include "hdyn.h"
#include <star/dstar_to_kira.h>

local bool remove_escapers(hdyn* b,
			   real rmax, vector lagr_pos, vector lagr_vel)
{
    bool correct_dynamics = false;
    real rmax2 = rmax*rmax;
    int n_esc = 0;

    for_all_daughters(hdyn, b, bi)
	if (square(bi->get_pos()-lagr_pos) > rmax2)
	    n_esc++;

    if (n_esc <= 0) {
	cerr << "\n  No escapers\n";
	return false;
    }

    cerr << endl << n_esc << " escaper(s):\n";

    hdyn** esc_list = new hdynptr[n_esc];
    
    real epot0, ekin0, etot0;
    calculate_energies_with_tidal(b, epot0, ekin0, etot0);
    
    n_esc = 0;
    for_all_daughters(hdyn, b, bj) {

	// Escape criterion (note that we do NOT check E > 0):
	
	if (square(bj->get_pos()-lagr_pos) > rmax2) {
	    
	    cerr << "    " << bj->format_label() << " " << bj->get_mass()
		 << endl;
	    cerr << "    pos: " << bj->get_pos()-lagr_pos << "   |pos| = "
		 << abs(bj->get_pos()-lagr_pos) << endl;
	    cerr << "    vel: " << bj->get_vel()-lagr_vel << "   |vel| = "
		 << abs(bj->get_vel()-lagr_vel) << endl;

	    if (b->get_use_sstar())
		if (has_sstar(bj)) {
		    cerr << "    ";  
		    put_state(make_star_state(bj));
		    cerr << endl;
		}
	    
//	    if (bj->get_starbase() != NULL) {
//		bj->get_starbase()->print_star_story(cerr);
//	    }

	    for_all_daughters(hdyn, b, bb) {
		if (bb->get_nn() == bj) {
		    cerr << "    reset NB for " << bb->format_label()<<endl;
		    bb->set_nn(NULL);
		}
	    }

	    // Correct the list of perturbed top_level binaries.

	    if (bj->is_perturbed_cm())
		bj->remove_from_perturbed_list();

	    esc_list[n_esc++] = bj;
	    detach_node_from_general_tree(*bj);

	    b->inc_mass(-bj->get_mass());	// Not done by detach_node...
	}
    }

    // MUST do this to avoid recoil eventually carrying the
    // entire system beyond the tidal radius!

    b->to_com();

    // Correct all perturber lists.

    correct_perturber_lists(b, esc_list, n_esc);

    // Should possibly recompute accs and jerks on top-level nodes here too.

#if 0
    calculate_acc_and_jerk_on_all_top_level_nodes(b);
#endif

    if (b->n_leaves() > 1) {

	real epot = 0, ekin = 0, etot = 0;
	calculate_energies_with_tidal(b, epot, ekin, etot);

	cerr << endl;
	PRI(2); PRC(epot0); PRC(ekin0); PRL(etot0);
	PRI(2); PRC(epot); PRC(ekin); PRL(etot);
    
	// pp3(cm, cerr);
    
	real de_total = etot - etot0;
    
	PRI(2); PRL(de_total); 
    
	// Update statistics on energy change components:
    
	b->get_kira_counters()->de_total += de_total;

	predict_loworder_all(b, b->get_system_time());	// Unnecessary?

	cerr << "initialize_system_phase2 called from remove_escapers\n";
	initialize_system_phase2(b, 4);
    }

    return true;
}



void check_and_remove_escapers(hdyn* b,
			       xreal& ttmp, hdyn** next_nodes,
			       int& n_next, bool& tree_changed)
{
    if (b->get_scaled_stripping_radius() <= 0) return;

    // Note: Definition of scaled_stripping_radius
    // 	 incorporates initial mass factor.

    real stripping_radius = b->get_scaled_stripping_radius()
				* pow(total_mass(b), 1.0/3.0);

    // Note also that this scaling ensures that omega
    // remains constant as the system evolves if the Jacobi
    // radius is identified with the stripping radius.

    real mass0 = total_mass(b);

    vector lagr_pos = 0;
    if (find_qmatch(b->get_dyn_story(), "lagr_pos"))
	lagr_pos = getvq(b->get_dyn_story(), "lagr_pos");

    vector lagr_vel = 0;
    if (find_qmatch(b->get_dyn_story(), "pos_type"))
	if(streq(getsq(b->get_dyn_story(), "pos_type"), "density center"))
	    if (find_qmatch(b->get_dyn_story(), "density_center_vel"))
		lagr_vel = getvq(b->get_dyn_story(), "density_center_vel");

    if (remove_escapers(b, stripping_radius, lagr_pos, lagr_vel)) {

	PRI(2); PRC(b->get_scaled_stripping_radius());
	PRL(stripping_radius);

	tree_changed = true;
	fast_get_nodes_to_move(b, next_nodes, n_next,
			       ttmp, tree_changed);
	tree_changed = true;

	cerr << "\n  New N = " << b->n_leaves()
	     <<  "  mass = " << total_mass(b) << endl;
	PRI(2); print_recalculated_energies(b);

	// Update statistics.

	b->get_kira_counters()->dm_escapers += mass0 - total_mass(b);
    }
}