#ifndef     STARLAB_HOP_H
#   define  STARLAB_HOP_H

/// @file hop.h  Data and classes for the ~HOP algorithm.

#include "dyn.h"
#include "vector"

real d_nn_sq, d_nn_sq2, d_nn_sq3;
static dyn *nn2, *nn3;

/// \a nearest_neighbor:  Combine a neighbor pointer and a distance.

class nearest_neighbor {
  public:
    dyn* nn;			///< nearest neighbor pointer
    real d_nn_sq;		///< distance to nearest neighbor
    nearest_neighbor() {
	nn = NULL;
	d_nn_sq = VERY_LARGE_NUMBER;
    }
};

/// \a cluster:  List of objects in a clump, as defined by HOP.

class cluster {
  protected:

    dyn *h;			///< Center (densest) node
    vector<dynptr> clump;	///< List of all nodes

    int nstar;			///< Number of stars in the list.
    real mass;			///< Total mass of stars on the list.
    vec pos;			///< Position of h.
    vec vel;			///< Velocity of h.

  public:

    cluster() {
	h = NULL;
	nstar = 0;
	mass = 0;
    }
    cluster(dyn *b, dyn *bi) {
	h = b;
	clump.push_back(bi);
	nstar = 1;
	mass = bi->get_mass();
	pos = b->get_pos();
	vel = b->get_vel();
    }
    ~cluster() {}

    dyn *get_h() {return h;}
    vector<dynptr>& get_clump() {return clump;}

    void set_h(dyn *b) {h=b;}
    void set_nstar(int n) {nstar = n;}
    void set_mass(real m) {mass = m;}
    void set_pos(vec p) {pos = p;}
    void set_vel(vec p) {vel = p;}

    void increment(dyn *bi) {
	clump.push_back(bi);
	nstar++;
	mass += bi->get_mass();
    }

    void put(ostream &s = cerr) {
	s << "Cluster (id= " << h->get_index() 
	  << "), N= " << nstar 
	  << ", M= " << mass << endl; 
	s << "   pos= " << pos << "   vel= " << vel << endl;
#if 0
	vector<dynptr>::iterator i;
	for (i = clump.begin(); i != clump.end(); i++)
	    s << (*i)->get_index() << " " << (*i)->get_mass() << endl;
#endif
    }
};

/// \a hop:  List of all clusters partitioning the system.

class hop {
  protected:

    int nn_search;		///< Search option.
    vector<cluster> cl;		///< List of clusters.

  public:

    hop() {}
    ~hop() {}

    void set_nn_search(int n) {nn_search = n;}

    vector<cluster>& get_cl() {return cl;}

    dyn *densest_nth_nn(dyn *b);
    void find_clump_center(dyn *b);
    void find_primary_cluster(dyn *b);
    void add_cluster_center(dyn* bc, dyn *bi);

    void put(ostream &s = cerr) {
	vector<cluster>::iterator ic;
	for (ic = cl.begin(); ic<cl.end(); ic++) {
	    ic->put(s);
	}
    }

    void put_snap(ostream &s) {
	vector<cluster>::iterator ic;
	for (ic = cl.begin(); ic<cl.end(); ic++) {
	    put_dyn(ic->get_h());
	}
    }
};

#endif    // STARLAB_HOP_H

