/* $Id$
 * $Date$
 * $Author$
 * $Revision$
 */

#include "lisasim-tdisignal.h"

TDIsignal::TDIsignal(LISA *mylisa, WaveObject *mywave) {
    phlisa = mylisa->physlisa();
    lisa = mylisa;

    wave = mywave;
}

void TDIsignal::setphlisa(LISA *mylisa) {
    phlisa = mylisa;
}

double TDIsignal::psi(Wave *nwave, Vector &lisan, double t) {
    // check if the Wave is active at time t
    if(!nwave->inscope(t)) return 0.0;

    Tensor cwave;
    nwave->putwave(cwave,t);
    
    Vector tmp;
    tmp.setproduct(cwave,lisan);

    return 0.5 * lisan.dotproduct(tmp);
}

// the y as computed below should now be fully covariant
// the time of evaluation (reception) is retarded according to the nominal lisa
// then the time of emission is retarded according to physical lisa
// the wave retardation for the emitting link is taken at the correct retarded time

// note that the call to putn is already computing the armlength that we request later
// we could make putn return the corresponding armlength

double TDIsignal::y(int send, int slink, int recv, int ret1, int ret2, int ret3, double t) {
    return y(send,slink,recv,ret1,ret2,ret3,0,0,0,0,t);
}

double TDIsignal::y(int send, int slink, int recv, int ret1, int ret2, int ret3, int ret4, int ret5, int ret6, int ret7, double t) {
    lisa->newretardtime(t);

    lisa->retard(ret7); lisa->retard(ret6); lisa->retard(ret5);
    lisa->retard(ret4); lisa->retard(ret3); lisa->retard(ret2); lisa->retard(ret1);

    double retardedtime = lisa->retardedtime();

    // for the moment, let's not trust the sign of slink, and recompute it

    int link = abs(slink);

    if( (link == 3 && recv == 2) || (link == 1 && recv == 3) || (link == 2 && recv == 1) )
	link = -link;

    // since the linkn returned by the "modern" versions of putn is oriented,
    // therefore the sign in denom is the same (-) for both positive and negative links
    // there's no problem in psi, because n is dotted twice into h

    Vector linkn;
    lisa->putn(linkn,link,retardedtime);    

    double retardsignal = retardedtime - phlisa->armlength(link,retardedtime);

    Vector psend, precv;
    phlisa->putp(psend,send,retardsignal);
    phlisa->putp(precv,recv,retardedtime);

    // loop over waves (if there is more than one)
    // using the WaveObject interface (firstwave, nextwave)

    Wave *nwave = wave->firstwave();
    if(!nwave) return 0.0;

    double accpsi = 0.0;

    do {
	double acc = ( psi(nwave, linkn, retardsignal - psend.dotproduct(nwave->k)) -
		       psi(nwave, linkn, retardedtime - precv.dotproduct(nwave->k)) );

	if(acc != 0.0) accpsi += acc / (1.0 - linkn.dotproduct(nwave->k));
    } while( (nwave = wave->nextwave()) );

    return accpsi;
}

// the next three names are not standard!

double TDIsignal::M(double t) {
    return( y(1, 2, 3, 2, 0, 0, t) -
            y(1, 3, 2, 3, 0, 0, t) + 
            y(3, 2, 1, 0, 0, 0, t) -
            y(2, 3, 1, 0, 0, 0, t) );
}

double TDIsignal::N(double t) {
    return( y(2, 3, 1, 3, 0, 0, t) -
            y(2, 1, 3, 1, 0, 0, t) + 
            y(1, 3, 2, 0, 0, 0, t) -
            y(3, 1, 2, 0, 0, 0, t) );
}

double TDIsignal::O(double t) {
    return( y(3, 1, 2, 1, 0, 0, t) -
            y(3, 2, 1, 2, 0, 0, t) + 
            y(2, 1, 3, 0, 0, 0, t) -
            y(1, 2, 3, 0, 0, 0, t) );
}

