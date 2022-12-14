
#include "stdio.h"
#include "scatter.h"

// set_merger_mass_and_radius: determine mass and radius of merger product

local void set_merger_mass_and_radius(sdyn * bn, sdyn * bi, sdyn * bj)
{
  bn->set_mass(bi->get_mass() + bj->get_mass());       // No mass loss
  bn->set_radius(bi->get_radius() + bj->get_radius()); // R \propto M
}


// set_merger_dyn: determine pos, vel, acc and jerk of merger product,
//                 and determine energy dissipation

local void set_merger_dyn(sdyn * bn, sdyn * bi, sdyn * bj)
{
    real  mi = bi->get_mass();
    real  mj = bj->get_mass();
    real  m_inv = 1/(mi + mj);
    
    bn->set_pos((mi*bi->get_pos() + mj*bj->get_pos())*m_inv);
    bn->set_vel((mi*bi->get_vel() + mj*bj->get_vel())*m_inv);
    bn->set_acc((mi*bi->get_acc() + mj*bj->get_acc())*m_inv);
    bn->set_jerk((mi*bi->get_jerk() + mj*bj->get_jerk())*m_inv);

    vec d_pos = bi->get_pos() - bj->get_pos();
    real rij = sqrt(d_pos*d_pos);

    vec d_vel = bi->get_vel() - bj->get_vel();
    real vij2 = d_vel*d_vel;

    real eij_pot = -mi * mj / rij;
    real eij_kin = 0.5 * mi * mj * m_inv * vij2;

    // Include tidal term from spectator star, if any:

    sdyn * b = bi->get_parent();
    sdyn * bk = NULL;

    for_all_daughters(sdyn, b, bb)
	if (bb != bi && bb != bj) bk = bb;

    real tidal_pot = 0;
    if (bk) {
	real mn = mi + mj;
	real mk = bk->get_mass();
	tidal_pot = -mn * mk / abs(bn->get_pos() - bk->get_pos())
		    + mi * mk / abs(bi->get_pos() - bk->get_pos())
		    + mj * mk / abs(bj->get_pos() - bk->get_pos());
    }

    bn->set_energy_dissipation(bi->get_energy_dissipation()
			       + bj->get_energy_dissipation()
			       + eij_pot + eij_kin
			       - tidal_pot);

}

local char * string_index_of_node(node * n)
{
    char * tmp;
    tmp = n->get_name();
    if (tmp != NULL) {
	return tmp;
    } else {
	static char integer_id[20];
	sprintf(integer_id, "%d", n->get_index());
	n->set_label(integer_id);
	return n->get_name();
    }
}

local char * construct_merger_label(node * ni, node * nj)
{
    static char new_name[1024];
    sprintf(new_name, "%s+%s",
	    string_index_of_node(ni),
	    string_index_of_node(nj));
    return new_name;
}

// merge: replace two particles by their center of mass.

void merge(sdyn * bi, sdyn * bj) {

    sdyn * b = bi->get_parent();
    if (b != bj->get_parent()) err_exit("merge: parent conflict...");

    // Note: any stories attached to the particles are lost.

    if (!bi->is_leaf() || !bj->is_leaf())  {
      cerr << "in merge: At least one of us is not a leaf!";
      exit(-1);
    }

    sdyn * bn = new sdyn();

#if 0
cerr.precision(6);
cerr << "entering merge(" << bi->get_index() << ", " << bj->get_index()
     << ") at dist = " << abs(bi->get_pos()-bj->get_pos()) 
     << " with r1 = "<<bi->get_radius() << " r2 = " << bj->get_radius() << endl
     << " with v1 = " << bi->get_vel() << "\n"
     << "  and v2 = " << bj->get_vel() << endl
     << "         at t = " << b->get_time() << endl;
#endif

    set_merger_mass_and_radius(bn, bi, bj);
    set_merger_dyn(bn, bi, bj);
    bn->set_time(bi->get_time());

    if(bi->get_mass()>bj->get_mass())
      bn->set_label(construct_merger_label(bi, bj));
    else
      bn->set_label(construct_merger_label(bj, bi));

    detach_node_from_general_tree(bi);
    detach_node_from_general_tree(bj);
  
    add_node(bn, b);
}

// merge_collisions: recursively merge any stars in contact.

real merge_collisions(sdyn * b)
{

  real de = 0;

    int coll = 1;
    while (coll) {

      if (b->n_leaves()<=2) {
	cerr << "Only two stars left in scatter_merge()"<<endl;
	cerr << "return from scatter_merge with de=0"<<endl;
	return de;
      }

	coll = 0;
	// For 3-body interactions using
	// for_all_daughters(sdyn, b, bi)
	// is correct, but for >3 body interactions one has to be more 
	// careful and use:
	for_all_leaves(sdyn, b, bi)
	    {
	    if (coll) break;
	    for (sdyn * bj = bi->get_younger_sister(); bj != NULL; 
		 bj = bj->get_younger_sister()) {

	      // Instead of 
	      //   if ( abs(bi->get_pos() - bj->get_pos())
	      // in scatter3 we better check the position relative to parent:
	      if (abs(something_relative_to_root(bi, &sdyn::get_pos)-something_relative_to_root(bj, &sdyn::get_pos))
		  < (bi->get_radius() + bj->get_radius()) ) {

#if 0
		  cerr << "In merge_collisions: a collision occured" << endl;
		  PRL(b->get_name());
		  PRC(bi->get_name());PRL(bj->get_name());
		  PRC(abs(bi->get_pos() - bj->get_pos()));
		  PRC(abs(something_relative_to_root(bi, &sdyn::get_pos)-something_relative_to_root(bj, &sdyn::get_pos)));
		  PRC(something_relative_to_root(bi, &sdyn::get_pos));PRL(bi->get_pos());
		  PRC(something_relative_to_root(bj, &sdyn::get_pos));PRL(bj->get_pos());
		  PRC(bi->get_radius());PRC(bj->get_radius());
		  PRL(bi->get_radius()+bj->get_radius());
		  PRC(bi->get_name());PRL(bj->get_name());
		  PRC(bi->n_leaves());PRL(bi->is_leaf());
		  PRC(bj->n_leaves());PRL(bj->is_leaf());
		  put_node(b);
#endif
	      
		  cerr << "Prior to b->flatten_node in merge_collisions" << endl;
		  b->flatten_node();
		  real kin, pot;
		  real etot_init = calculate_energy_from_scratch(b, kin, pot);
		  cerr << "Prior to make_tree in merge_collisions" << endl;
		  make_tree(b, 1, 1, 2, false);
		  cerr << "After make tree, but before merge"<<endl<<flush;

		  merge(bi, bj);
		  cerr << "Post merge in merge_collisions" << endl;
		  b->flatten_node();
		  cerr << "Post b-flatte_node in merge_collisions" << endl;
		  real etot_final = calculate_energy_from_scratch(b, kin, pot);
		  make_tree(b, 1, 1, 2, false);
		  real merger_de = etot_final - etot_init;
		  de = merger_de;

		  coll = 1;
		  cerr << "Break out with detected collision."<<endl;
		  break;
		}
	    }
	}
    }

    return de;
}
