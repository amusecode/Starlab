
       //=======================================================//   _\|/_
      //  __  _____           ___                    ___       //     /|\ ~
     //  /      |      ^     |   \  |         ^     |   \     //         _\|/_
    //   \__    |     / \    |___/  |        / \    |___/    //           /|\ ~
   //       \   |    /___\   |  \   |       /___\   |   \   // _\|/_
  //     ___/   |   /     \  |   \  |____  /     \  |___/  //   /|\ ~
 //                                                       //           _\|/_
//=======================================================//             /|\ ~

// hdyn_inline.C:  Inline functions for use in hdyn_ev.C, hdyn_grape4/6.C,
//		   and hdyn_slow.C.
//
//		   These functions tend to be used frequently, at low
//		   levels in the code.  For efficiency, include this file
//		   directly into the source files, rather than placing
//		   the functions in libraries.  As the number of functions
//		   increases, maybe we should place some in libraries, and
//		   include directly only the essentials...  (Steve, 10/04)
//
// Local inline functions defined:
//
//	void update_nn_coll
//	real define_perturbation_radius_factor
//	bool is_perturber
//	real binary_scale
//	real compute_sum_of_radii
//
//----------------------------------------------------------------------

// inline void update_nn_coll
//	update various neighbor pointers associated with a node.

local inline void update_nn_coll(hdyn *this_node, int id,
				 real d_to_b_sq, hdyn *b,
				 real &d_nn_sq, hdyn *&nn,
				 real sum_of_radii,
				 real &d_coll_sq, hdyn *&coll)

// Note: arguments "this_node" and "id" are needed only for possible
// 	 diagnostic output.

{
    if (d_to_b_sq < d_nn_sq) {

#if 0
	hdyn *old_nn = nn;
	real old_d_nn_sq = d_nn_sq;
#endif

	d_nn_sq = d_to_b_sq;
	nn = b;


#if 0
	if (this_node->get_real_system_time() > 13.62265) {
	    cerr << "update_nn_coll(" << id << "): ";
	    PRL(this_node->format_label());
	    PRI(4); PRC(old_nn); PRL(sqrt(old_d_nn_sq));
	    if (old_d_nn_sq < 0.5*VERY_LARGE_NUMBER) {
		PRI(4); cerr << "old: " << flush;
		if (old_nn->is_valid()) {
		    PRC(old_nn->format_label());
		}
		if (old_d_nn_sq > 0) {
		    PRC(sqrt(old_d_nn_sq));
		}
		cerr << endl << flush;
	    }
	    PRI(4); PRC(nn->format_label()); PRL(sqrt(d_nn_sq));
	}
#endif

    }

    real coll_dist = d_to_b_sq - sum_of_radii*sum_of_radii;

    if (coll_dist < d_coll_sq) {
	d_coll_sq = coll_dist;
	coll = b;
    }
}


// Set/check criteria for placing bi on the perturber list of a node.
// Note that these functions are the *only* places in kira where this
// criterion is defined.

#define PERTURBER_CRITERION	2	// For determination of perturbers
					//   0:  distance only
					//   1:  perturbation (includes mass)
					//   2:  hybrid

// inline real define_perturbation_radius_factor()
// inline bool is_perturber()

// Simple distance criterion for placing bi on perturber list is
//
//	    distance_squared  <  perturbation_radius^2
//
// where
//
//	    perturbation_radius  =  gamma^{-1/3} * r_bin
//
// and
//
//	    r_bin  =  max(a, rij).
//
// (Note: we used to have r_bin  =  min(d_min, max(a, rij)), which is OK if
//  rij is never greater than d_min, but this may not always be the case.)
//
// This is OK for roughly equal-mass binaries, but it doesn't take
// masses into account -- IMPORTANT when a wide mass range exists.
// (Also, it is not clear that d_min is the correct upper limit on
// the "size" of the binary r_bin.)
//
// The criterion including masses, which reduces to the above criterion
// for equal masses, is
//
//	      2 * m_pert * r_bin	       M_bin
//	    ---------------------  =  gamma * -------
//	    perturbation_radius^3	      r_bin^2
//
// or
//
//	    perturbation_radius  =  gamma^{-1/3} * r_bin
//	    				* (2 * m_pert / M_bin)^{1/3}.
//
// Note that, in the "perturbation" definitions, we work with cubed radii
// (to avoid cube roots in the loop that computes the perturber list).
// In these cases, "perturbation_radius_factor" is (apart from the mass
// factor) perturbation_radius^3.
//
// Also, in these cases, pert_factor_sq is really gamma2^{-1/3} = gamma^{-2/3}.
// All we really need is 1/gamma!  FIX THIS SOMEDAY!!
//
//*** Distance criterion is fine for perturbers of mass comparable to or
//*** less than the masses of the binary components.
//*** Hybrid criterion uses a fixed perturber radius, but then also includes
//*** more distant massive stars.
//
// Use the same criterion for softened systems (relevant when eps <<
// d_min), even though the details are a bit suspect...  (Note that
// the node combination criterion does include eps.)

// binary_scale:  compute an appropriate "size" for a binary.

local inline real binary_scale(hdyn* cm)
{
    static hdyn *last_cm = NULL;
    static xreal last_time = 0;
    static real last_scale = 0;

    xreal time = cm->get_time();

    if (cm == last_cm
	&& time == last_time)
	return last_scale;

    hdyn *d1 = cm->get_oldest_daughter();
    if (!d1) return 0;

    hdyn *d2 = d1->get_younger_sister();
    if (!d2) return 0;

    real sep = abs(d1->get_pos() - d2->get_pos());
    real energy = 0.5 * square(d1->get_vel() - d2->get_vel())
			- cm->get_mass() / sep;
    real sma = 0.5 * cm->get_mass() / abs(energy);

    last_cm = cm;
    last_time = time;
    last_scale = Starlab::max(sma, sep);

    //  =  max(2*sma, sep);	// Better? -- more conservative, and also
				// avoids discontinuous changes in perturber
				// lists as the binary phase changes.
				// -- changes default gamma.

#ifdef T_DEBUG
    real sys_t = cm->get_system_time();
    if (IN_DEBUG_RANGE(sys_t) && T_DEBUG_LEVEL > 0) {
	cerr << "DEBUG: in binary_scale for node "
	     << cm->format_label() << endl << flush;
	PRI(4); PRC(cm->get_mass()); PRC(sep); PRC(energy); PRL(sma);
    }
#endif

    return last_scale;
}

// define_perturbation_radius_factor:
//		 define the value of perturber_radius_squared.

local inline real define_perturbation_radius_factor(hdyn *b,	// 'this' node
						    real pert_factor_sq)
{
    int  perturber_criterion = b->get_kira_options()->perturber_criterion;
    // real d_min_sq = b->get_d_min_sq();
    real r_bin = binary_scale(b);
    real m_bin = b->get_mass();

    // Note that pert_factor_sq will be gamma23 in normal use, but
    // it may in general take any value.

    // NOTE: removed "min(d_min_sq..." lines (Steve, 12/01).

    if (perturber_criterion == 0) {

	// Distance criterion:

	// return pert_factor_sq * min(d_min_sq, r_bin * r_bin);
	return pert_factor_sq * r_bin * r_bin;

    } else if (perturber_criterion == 1) {


	// real rp2 = pert_factor_sq * min(d_min_sq, r_bin * r_bin);
	real rp2 = pert_factor_sq * r_bin * r_bin;

	// return 2 * pow(rp2, 1.5) / m_bin;
	return 2 * rp2 * sqrt(rp2) / m_bin;

    } else if (perturber_criterion == 2) {

	// Hybrid criterion for perturbation radius is the distance
	// criterion, but storing rp^3 instead of rp^2; alternatively,
	// it is the perturbation criterion without the mass factor:

	// real rp2 = pert_factor_sq * min(d_min_sq, r_bin * r_bin);
	real rp2 = pert_factor_sq * r_bin * r_bin;

	// return pow(rp2, 1.5);
	return rp2 * sqrt(rp2);
    }
}

// perturbation_scale_sq: return a characteristic squared scale for the
//			  radius of b's perturber sphere (Steve, 12/01)

local inline real perturbation_scale_sq(hdyn *b,
					real pert_factor_sq)
{
    // Decode the perturber criterion and return the radius that would
    // be applied for a perturber of mass equal to half the mass of b.
    // Basically, we just repeat the action of the previous function,
    // but return rp2.  Keep the function here so we can maintain
    // consistency if the criteria change.  The pert_factor_sq used
    // here must be the same as that sent to the previous function.
    //
    // Note that we end up calling binary_scale TWICE -- once in the
    // previous function, and again here.  To be fixed...

    if (!b->get_oldest_daughter()) return 0;

    real r_bin = binary_scale(b);
    return pert_factor_sq * r_bin * r_bin;
}

local inline real perturber_rescale(hdyn *b, real fac)
{
    // Estimate the scaling of perturbation_radius_factor to reduce the
    // neighbors enclosed by a factor of fac (< 1), taking into account
    // the possible different values of perturber_criterion.

    int  perturber_criterion = b->get_kira_options()->perturber_criterion;

    if (perturber_criterion == 0) {

	// Perturbation_radius_factor ~ distance squared.

	if (fac > 0) fac = pow(fac, 2./3);

    }  else {

	// Perturbation_radius_factor ~ distance cubed.  No action needed.

    }

    return fac;
}

// is_perturber: check the perturbation criterion.
//
// (Note that we implicitly assume eps = 0 here.  This underestimates
// the number of perturbers by overestimating the binary interaction.
// 							SLWM, 5/07)

local inline bool is_perturber(hdyn *b,			     // 'this' CM node
			       real m_pert,		     // perturber mass
			       real distance_squared,
			       real perturbation_radius_factor)
{
    int  perturber_criterion = b->get_kira_options()->perturber_criterion;
    real m_bin = b->get_mass();

    if (perturber_criterion == 0) {

	// Distance criterion:

	return (distance_squared < perturbation_radius_factor);

    } else if (perturber_criterion == 1) {

	// Perturbation criterion:

	return (distance_squared * sqrt(distance_squared)
					< m_pert * perturbation_radius_factor);

    } else if (perturber_criterion == 2) {

	// Hybrid criterion is distance criterion, plus additional
	// massive perturbers:

	real d3 = distance_squared * sqrt(distance_squared);

	if (m_pert <= 0.5*m_bin)
	    return (d3 < perturbation_radius_factor);		// low-mass

	return (0.5*m_bin*d3					// high-mass
		     < m_pert * perturbation_radius_factor);

    }
}

local inline real crit_separation_cubed(hdyn *b,	// binary CM node
					real m_pert,	// perturber mass
					real r_bin,	// binary scale
					real gamma)	// perturbation
{
    real m_bin = b->get_mass();
    int  perturber_criterion = b->get_kira_options()->perturber_criterion;

    // real d_min_sq = b->get_d_min_sq();
    // real d2 = Starlab::min(d_min_sq, r_bin * r_bin);
    real d2 = r_bin * r_bin;
    real d3 = d2 * sqrt(d2) / gamma;

    if (perturber_criterion == 0
	|| (perturber_criterion == 2 && m_pert <= 0.5 * m_bin))

	return d3;

    else

	return 2 * (m_pert / m_bin) * d3;
}

// crit_separation_cubed: return the critical separation for the
//			  adopted perturbation criterion.

local inline real time_to_radius(real dr,	// distance to cover
				 real vr,	// relative radial velocity
				 real ar)	// relative radial acceleration
{
    // Estimate the time required for two particles, of given relative
    // velocity and acceleration, to decrease their separation by dr.
    // Use the following time scales:
    //
    //		t1  =  dr / |vr|		("crossing" time)
    //		t2  =  sqrt (2 * dr / |ar|)	("free-fall" time)
    //
    // If the particles are in the same clump, then ar is the two-body
    // acceleration and t2 really is the free-fall time.  Otherwise,
    // ar is the relative acceleration in the global cluster field.

    if (dr <= 0) return 0;

    real dt = VERY_LARGE_NUMBER;

    if (vr < 0) dt = -dr / vr;
    if (ar < 0) dt = Starlab::min(dt, sqrt (-2 * dr / ar));

    return dt;
}



// It is most efficient to separate the functionalities of functions
// define_perturbation_radius_factor() and is_perturbed() for computation
// of many perturbations in a loop.  However, it is very convenient to
// have functions which compute and/or test the perturbation in a single
// call.  Don't worry about efficiency here -- packaging is more important.

local inline bool is_perturber(hdyn *cm, hdyn *p, real gamma)
{
    // Is node p a perturber of the binary with center of mass cm
    // at the level gamma?

    real pert_radius_factor
	= define_perturbation_radius_factor(cm, pow(gamma, -2/3.));
    real distance_squared
	= square(hdyn_something_relative_to_root(cm, &hdyn::get_pos)
		  - hdyn_something_relative_to_root(p, &hdyn::get_pos));
    return is_perturber(cm, p->get_mass(),
			distance_squared, pert_radius_factor);
}

local inline real estimated_perturbation(hdyn *cm, hdyn *p)
{
    // Return the perturbation on the binary with center of mass cm
    // due to perturber p.  (Consistent with is_perturber().)

    // PRC(cm); PRL(p);
    real m_bin = cm->get_mass();
    real m_pert = p->get_mass();
    int  perturber_criterion = cm->get_kira_options()->perturber_criterion;
    // PRL(perturber_criterion);

    real r_bin = binary_scale(cm);
    real d2 = r_bin * r_bin;
    // PRL(r_bin);
    real distance_squared
	= square(hdyn_something_relative_to_root(cm, &hdyn::get_pos)
		  - hdyn_something_relative_to_root(p, &hdyn::get_pos));
    // PRL(sqrt(distance_squared));

    real gamma = pow(d2/distance_squared, 1.5);
    if (perturber_criterion == 1
	|| (perturber_criterion == 2 && m_pert > 0.5 * m_bin))
	gamma *= 2 * m_pert / m_bin;

    return gamma;
}

local inline real compute_sum_of_radii(hdyn *bi, real radi, bool bi_is_bh,
				       hdyn *bj, real radj, bool bj_is_bh)
{
    real sum_of_radii = radi + radj;

    // For black hole, use tidal radius instead of event horizon radius.

    if (bi_is_bh) {

	// bi is a black hole.

        if (!bj_is_bh) 
            sum_of_radii = 2 * radj
                * (pow(bi->get_mass()/bj->get_mass(), ONE_THIRD));

    } else if (bj_is_bh)

        sum_of_radii = 2 * radi
            * (pow(bj->get_mass()/bi->get_mass(), ONE_THIRD));

    return sum_of_radii;
}
