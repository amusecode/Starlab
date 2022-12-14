
#include "hdyn.h"
#include <star/dstar_to_kira.h>
#include <star/single_star.h>
#include "hdyn_inline.C"

// NOTE: The following function may be called repeatedly from within
// the innermost force-evaluation loop (in hdyn_ev.C, hdyn_grape4.C, and
// hdyn_grape6.C), so we really don't want to call find_qmatch() every time.
// Retain the functionality of allowing story checking, but suppress it
// by default.  If it is important to have the posibility of a black hole
// not identifiable by using get_element_type() (e.g. for a calculation
// without stellar evolution), then this information may have to become
// part of the hdyn class structure.  The "get_sum..." function is now
// just a wrapper for the inlined version.  (Rewritten by Steve, 1/05) 

real get_sum_of_radii(hdyn* bi, hdyn* bj,
		      bool check_story)		// default = false
{
    // Interpret the radii.  Note that this code is replicated in
    // function flat_calculate_acc_and_jerk(), in hdyn_ev.C

    real radi = bi->get_radius();
    bool bi_is_bh = false;
    if (radi < 0) {
	bi_is_bh = true;
	radi = -radi;
    }
    if (!bi_is_bh && check_story)
	bi_is_bh = getiq(bi->get_log_story(), "black_hole");

    real radj = bj->get_radius();
    bool bj_is_bh = false;
    if (radj < 0) {
	bj_is_bh = true;
	radj = -radj;
    }
    if (!bj_is_bh && check_story)
	bj_is_bh = getiq(bj->get_log_story(), "black_hole");

    return compute_sum_of_radii(bi, radi, bi_is_bh, bj, radj, bj_is_bh);
}

local real conv_v_dyn_to_star(real v, real rf, real tf) {

//              Internal velocity is km/s
//              Internal size is solar raii
//              Internal time is Myr.
//              km/s to Rsun/Myr
//      real to_Rsun_Myr = cnsts.physics(km_per_s) * cnsts.physics(Myear)
//	               / cnsts.parameters(solar_radius);

      real myear = 3.15e+13;
      real km_s = 1.0e+5;
      real R_sun = 6.960e+10;
      real to_Rsun_Myr = km_s * myear / R_sun;
      real to_dyn      = rf/tf;
      
      return v/(to_Rsun_Myr * to_dyn);
   }

real print_encounter_elements(hdyn* bi, hdyn* bj,
			      const char* s,	    // default = "Collision"
			      bool verbose)	    // default = true
{
    kepler k;
    initialize_kepler_from_dyn_pair(k, bi, bj, true);	// minimal kepler
							// -- no phase info

    // Note from Steve (9/04):  If the energy is positive, we only want
    // to print the information once.  Apparently it is possible for
    // "wiggles" in the orbit (or imprecise data storage?) to cause this
    // function to be called several times.  Address this by printing
    // only if bi->time and bj->time are less than the time needed to
    // separate to twice the current radius (set below).

    bool print = true;
    real t = bi->get_time();

    if (k.get_energy() > 0)
	if (find_qmatch(bi->get_log_story(), "coll_tskip")
	    && find_qmatch(bj->get_log_story(), "coll_tskip")) {
	    real ti = getrq(bi->get_log_story(), "coll_tskip");
	    real tj = getrq(bj->get_log_story(), "coll_tskip");
	    if (ti < VERY_LARGE_NUMBER && tj < VERY_LARGE_NUMBER    // overkill
		&& t < ti || t < tj) print = false;
	}

    if (print) {
	cerr << endl;
	cerr << s << " at time = " << bi->get_time();

	if (bi->get_use_sstar() && verbose) {

	    cerr << " ("
		 << bi->get_starbase()->conv_t_dyn_to_star(bi->get_time())
		 << " [Myr])";
	    cerr << " between \n"
		 << "     " << bi->format_label() << " (";
	    put_state(make_star_state(bi), cerr);
	    cerr << "; M = " << bi->get_starbase()->get_total_mass()
		 << " [Msun], "
		 << " R = " << bi->get_starbase()->get_effective_radius()
		 << " [Rsun]) and\n"
			    << "     " << bj->format_label()
				<< " (";
	    put_state(make_star_state(bj), cerr);
	    cerr << "; M = " << bj->get_starbase()->get_total_mass()
		 << " [Msun], "
		 << " R = " << bj->get_starbase()->get_effective_radius()
		 << " [Rsun]) "
		 <<"\n     at distance "
		 << k.get_periastron()
		 << " ("
		 << bi->get_starbase()->conv_r_dyn_to_star(k.get_periastron())
		 << " [Rsun])"
		 <<" and velocity "
		 << k.get_rel_vel()
	      	 << " ("
		 << conv_v_dyn_to_star(abs(k.get_rel_vel()),
			       bi->get_starbase()->conv_r_star_to_dyn(1),
			       bi->get_starbase()->conv_t_star_to_dyn(1))
	         << " [km/s])." << endl;
	    cerr << "  Primary position: " << bi->get_pos() << endl;
	    cerr << "         Dpos: " << bi->get_pos()-bj->get_pos() 
		 << " (" << bi->get_starbase()->conv_r_dyn_to_star(bi->get_pos()[0]-bj->get_pos()[0]) 
		 << ", " << bi->get_starbase()->conv_r_dyn_to_star(bi->get_pos()[1]-bj->get_pos()[1]) 
		 << ", " << bi->get_starbase()->conv_r_dyn_to_star(bi->get_pos()[2]-bj->get_pos()[2]) << " [Rsun])." 
		 << endl;
	    cerr << "  Primary velocity: " << bi->get_vel() << endl;
	    cerr << "         Dvel: " << bi->get_vel()-bj->get_vel() 
		 << " (" << conv_v_dyn_to_star(bi->get_vel()[0]-bj->get_vel()[0],
				   bi->get_starbase()->conv_r_star_to_dyn(1),
				   bi->get_starbase()->conv_t_star_to_dyn(1)) 
		 << ", " << conv_v_dyn_to_star(bi->get_vel()[1]-bj->get_vel()[1],
				   bi->get_starbase()->conv_r_star_to_dyn(1),
				   bi->get_starbase()->conv_t_star_to_dyn(1))
		 << ", " << conv_v_dyn_to_star(bi->get_vel()[2]-bj->get_vel()[2],
				   bi->get_starbase()->conv_r_star_to_dyn(1),
				   bi->get_starbase()->conv_t_star_to_dyn(1)) 
		 << " [km/s])." << endl;
	} else {

	    cerr << " between " << bi->format_label();
	    if(bi->get_starbase()->get_element_type() == Black_Hole ||
	       (find_qmatch(bi->get_log_story(), "black_hole") &&
		getiq(bi->get_log_story(), "black_hole")==1))
		cerr << " [bh] ";
      
	    cerr << " and " << bj->format_label();
	    if(bj->get_starbase()->get_element_type() == Black_Hole ||
	       (find_qmatch(bj->get_log_story(), "black_hole") &&
		getiq(bj->get_log_story(), "black_hole")==1))
		cerr << " [bh] ";
	    cerr << "\n     at distance " << k.get_periastron()
		 <<" and velocity " << k.get_rel_vel();

      
	}

	cerr << endl;

	if (k.get_energy() < 0) {
	    cerr << "     Orbital parameters: ";

	    // (Don't bother with global distance and speed variables.)

	    print_binary_params(&k, bi->get_mass(), 0.0, 0.0, 0.0,
				abs(bi->get_pos()), verbose, 10, 10);
	    cerr << endl;
	}
	else {
	    cerr << "     E = " << k.get_energy() << endl;
	    real coll_tskip = k.pred_advance_to_radius(2*k.get_periastron());
	    // PRL(coll_tskip);
	    putrq(bi->get_log_story(), "coll_tskip", coll_tskip);
	    putrq(bj->get_log_story(), "coll_tskip", coll_tskip);
	    putrq(bi->get_log_story(), "coll_tskip", coll_tskip);
	}
    }

    return k.get_energy();
}


void check_print_close_encounter(hdyn *bi)
{
    if (bi && bi->is_valid()) {

        hdyn *nn = bi->get_nn();

	// Use nearest neighbor for encounter diagnostics.
	// Only consider encounters between nodes at the top level
	// or the next level down (added by Steve 4/99 to prevent
	// multiple output in bound hierarchical systems).

#if 0
	PRC(nn); PRC(bi->is_leaf());
	PRC(bi->get_parent()->is_top_level_node()); PRC(bi->get_kepler());
	PRC(bi->get_nn()); PRC(nn->is_valid()); PRC(nn->is_leaf());
	PRC(bi->is_top_level_node()); PRC(nn->is_top_level_node());
	PRC(nn->get_parent()->is_top_level_node()); PRC(nn->get_kepler());
	PRC(bi->get_d_nn_sq());
	PRL(bi->get_stellar_encounter_criterion_sq());
#endif
	if (nn && bi->is_leaf()
	    && (bi->is_top_level_node()
		|| bi->get_parent()->is_top_level_node())
	    && bi->get_kepler() == NULL
	    && bi->get_nn() != NULL
	    && nn->is_valid()
	    && nn->is_leaf()
	    && (nn->is_top_level_node()
		|| nn->get_parent()->is_top_level_node())
	    && nn->get_kepler() == NULL
	    && bi->get_d_nn_sq()
	    		<= bi->get_stellar_encounter_criterion_sq())

		print_close_encounter(bi, nn);
    }
}

// Flag close encounter distances between stars.  Output occurs on any
// unbound encounter, and on the first encounter in a bound system.
// Self-contained function, using the log story to propogate data.

void print_close_encounter(hdyn* bi,
			   hdyn* bj)
{
    if (bi->get_mass() < bj->get_mass())  // Note: this "<" means that equal-
	return;				  // mass stars get printed twice!

    real d2cc_2 = -1;
    real d2cc_1 = -1;
    real d2cc   = -1;

    // Variables:	bi is primary, bj is current coll
    //			cc_name is label of previous coll
    //			cc_time is time of previous coll
    //			d2cc is squared distance to coll
    //			d2cc_1 is previous d2cc
    //			d2cc_2 is previous d2cc_1
    //			pcc_name is label of coll at last pericenter
    //			pcc_time is time of last pericenter
    //			pcc_cntr counts bound pericenters
    //
    // All but bi and bj are stored in the bi log story.

    char *cc_name = getsq(bi->get_log_story(), "cc_name");

#if 0
    cerr << "\nIn print_close_encounter with bi =  " << bi->format_label()
	 << "  at time " << bi->get_system_time() << endl;
    cerr << "     "; print_coll(bi,2);
    cerr << "     "; print_nn(bi,2); 
    cerr << "     (bj = " << bj->format_label();
    cerr << ":  coll = "; print_coll(bj);
    cerr << ",  nn = "; print_nn(bj); cerr << ")" << endl;

    cerr << "     cc_name = " << getsq(bi->get_log_story(), "cc_name")
	 << endl;
#endif
  
    if (cc_name && streq(cc_name, bj->format_label())) {

	// Retrieve coll information from the log story.

	d2cc_2 = getrq(bi->get_log_story(), "d2cc_1");
	d2cc_1 = getrq(bi->get_log_story(), "d2cc");
	d2cc   = square(bi->get_pos() - bj->get_pos());

	if (d2cc_2 > d2cc_1 && d2cc >= d2cc_1) {    // just passed pericenter

	    int pcc_cntr = 0;
	    real E = get_total_energy(bi, bj);

	    if (E > 0) {

	        // Always print unbound encounter elements.

		if (bi->get_kira_diag()->report_close_encounters >= 1)
		    print_encounter_elements(bi, bj, "Close encounter");

	    }
	    else {

	        // Only print first bound encounter.

		char *pcc_name = getsq(bi->get_log_story(), "pcc_name");
	
		if (pcc_name && streq(pcc_name, bj->format_label())) {
		    pcc_cntr = getiq(bi->get_log_story(), "pcc_cntr");
		    if (pcc_cntr == -1) 
			pcc_cntr=0;
		}

		pcc_cntr++;

		if (pcc_cntr == 1
		    && bi->get_kira_diag()->report_close_encounters >= 2)
		    print_encounter_elements(bi, bj, "First bound encounter");
	    }

	    // Save data on last pericenter (bound or unbound).
            // Note that an unbound pericenter resets pcc_cntr to zero.

	    putiq(bi->get_log_story(), "pcc_cntr", pcc_cntr);
	    putsq(bi->get_log_story(), "pcc_name", bj->format_label());
	    putrq(bi->get_log_story(), "pcc_time", bi->get_time());  

	}

    }
    else if (getsq(bi->get_log_story(), "pcc_name"))

	if ((cc_name == NULL
	    || strcmp(cc_name, getsq(bi->get_log_story(), "pcc_name")))
	    && bi->get_kira_diag()->report_close_encounters > 0) {

	    // Unlikely multiple encounter(?).  Has been known to occur
	    // when one incoming star overtakes another.

	    cerr << endl << "print_close_encounter:  "
	         << "bi = " << bi->format_label()
		 << " at time " << bi->get_system_time() << endl;
	    cerr << "     current coll = " << bj->format_label();
	    cerr << "  previous coll = ";
	    if (cc_name)
	        cerr << cc_name;
	    else
	        cerr << "(null)";
	    cerr << endl;
	    cerr << "     coll at last pericenter = "
	         << getsq(bi->get_log_story(), "pcc_name");
	    cerr << "  (periastron counter = "
	         << getiq(bi->get_log_story(), "pcc_cntr") << ")\n";
	}

    putsq(bi->get_log_story(), "cc_name", bj->format_label());
    putrq(bi->get_log_story(), "d2cc_1", d2cc_1);  
    putrq(bi->get_log_story(), "d2cc", d2cc);  
    putrq(bi->get_log_story(), "cc_time", bi->get_time());  

#if 0
    cerr << "\nAt end of print_close_encounter at time "
	 << bi->get_system_time() << endl;
    cerr << "bi = " << bi->format_label();
    cerr << ", cc_name = " << getsq(bi->get_log_story(), "cc_name") << endl;
#endif

}

