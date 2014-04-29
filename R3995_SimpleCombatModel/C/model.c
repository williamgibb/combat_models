/* delayrb_r: Lanchester model with reinforcement and withdrawal
 *
 * Dewar and Gillogly:   1988-1990
 */

/* Code from Appendix A of the RAND Corporation R3995-RC paper
They own the copywrite, not me  */

/* XXX for readability, augmented assignment operations have been replaced 
with full expressions */

char *hdr = "$Id$"; // XXX what is this for?

#include <stdio.h>
#include <stdlib.h> /* XXX OSX complains about stdlib being missing */
#include <math.h>

typedef double real; // Use 'real' in place of 'double'

// XXX Global / Configurations values

real RI = 1000;         /* Initial number of Red troops */
real BI = 500;          /* Initial number of Blue troops */

real c1 = 1.0 / 2048.0; /* Defenders attrition rate  */
real c2 = 1.0 / 512.0;  /* Attackers " " */

real rBA = 4;           /* Blue calls for reinforcements at 1:4 */
real rRA = 2.5;         /* Read calls for reinforcements at 2.5:1 */

real rBW = 10;          /* Blue withdrawls at 1:10 */
real rRW = 1.5;         /* Red withdrawls at 1.5:1 */

real rBAA = 0.80;       /* Blue calls for more if attrition is high */
real rRAA = 0.80;       /* Red " */

real rBAW = 0.70;       /* Blue withdrawls if down to this */
real rRAW = 0.70;       /* Red " */

int B_delay = 70;       /* Arrival time delay of reinforcements */
int R_delay = 70;       /* Arrival time delay of reinforcements */

int B_maxchunks = 5;    /* How many chunks may they use? */
int R_maxchunks = 5;    /* How many chunks may they use? */

int b_tot_reinf = 1500; /* Total reinforcements */
int r_tot_reinf = 1500;

// XXX Global values used when running the simulation

real a1;                /* Reinforcement chunk size for Blue */
real a2;                /* "             "      "    "  Red  */

int bchunks, rchunks;   /* How many chunks have Blue and Red ordered? */

real B_reinf, R_reinf;  /* Total reinforcements tossed in */

int B_step, R_step;     /* Arrival time step of reinforcements */
int B_ordered, R_ordered;   /* How many reinforcements are ordered */

real B_with_thresh, R_with_thresh, B_reinf_thresh, R_reinf_thresh;

real rc;                /* Force ration: Red/Blue */
real R, B;              /* Current number of Red/Blue troops */

#define YES 1
#define NO 0
#define STOP 1
#define GO 0

#define MAXITER 200000

void call_reinforcements(i)  /* See if we need to call for reinforcements */
int i; /* Current time step */
{
    /* XXX - Uses the previously defined global variables */
    
    /* Blue reinforcement based on attrition */
    if (B_ordered == 0 && bchunks < B_maxchunks && B < B_reinf_thresh)
    {
        B_step = i + B_delay;
        B_ordered = a1;
        bchunks++; /* XXX YAY post-increment operator vomit */
        printf("Blue calls for reinforcements /A at %d.\n", i);
    }
    /* Blue reinforcements based on force ratio */
    if (B_ordered == 0 && bchunks < B_maxchunks && rc > rBA)
    {
        B_step = i + B_delay;
        B_ordered = a1;
        bchunks++; /* XXX YAY post-increment operator vomit */
        printf("Blue calls for reinforcements /FR at %d.\n", i);
    }
    /* Red reinforcement based on attrition */
    if (R_ordered == 0 && rchunks < R_maxchunks && B < R_reinf_thresh)
    {
        R_step = i + R_delay;
        R_ordered = a2;
        rchunks++; /* XXX YAY post-increment operator vomit */
        printf("Red calls for reinforcements /A at %d.\n", i);
    }
    /* Red reinforcements based on force ratio */
    if (R_ordered == 0 && rchunks < R_maxchunks && rc < rRA)
    {
        R_step = i + R_delay;
        R_ordered = a2;
        rchunks++; /* XXX YAY post-increment operator vomit */
        printf("Red calls for reinforcements /FR at %d.\n", i);
    }
}

void reinforce(i)    /* Do the reinforcement */
int i;
{
    /* XXX - Uses the previously defined global variables */
    
    if (B_ordered > 0 && B_step <= i)
    {
        B = B + B_ordered;
        B_reinf = B_reinf + B_ordered;
        B_ordered = 0;
        printf("Blue reinforcements arrive at %d.\n", i);
    }
    if (R_ordered > 0 && R_step <= i)
    {
        R = R + R_ordered;
        R_reinf = R_reinf + R_ordered;
        R_ordered = 0;
        printf("Red reinfocements arrive at %d.\n", i);
    }
}

int withdraw(i)     /* See if one side wants to withdraw */
int i;
{
    /* Check for withdrawal based on force ration or attr */
    
    /* XXX - Uses the previously defined global variables */
    /* XXX What if BOTH of the sides would withdraw in the
    same timestep?  The original logic defaults to the Blue
    team withdrawling first, but this is not a very good
    model since it has a built in bias instead of a third
    outcome. */
    
    if (B < B_with_thresh || rc > rBW)
    {
        /* XXX No status indicated as to WHY the withdrawl is occuring */
        printf("Blue withdrawls at %d.\n", i);
        return STOP;
    }
    if (R < R_with_thresh || rc < rRW)
    {
        printf("Red withdrawls at %d.\n", i);
        return STOP;
    }
    return GO;
}


void attrit()  /* Theow stones at frogs in sport */
{
    /* XXX - Uses the previously defined global variables */
    /* XXX - The use of b as a temporary store of the global
    value B is not entirely clear here, as it is derived from
    the formulae used to define the model in the R3995 paper */
    real b, t;
    
    b = B;          /* B and R troop strength */
    t = c1 * R;     /* Blue attrition */
    B = B - t;
    t = c2 * b;     /* frogs die in earnest */
    R = R - t;
}

int main(){
    long i;
    
    B = BI;
    R = RI;
    
    B_reinf = R_reinf = 0;  /* Total reinforcements so far */
    B_ordered = R_ordered = 0;
    
    B_with_thresh = BI * rBAW;
    B_reinf_thresh = BI * rBAA;
    
    R_with_thresh = RI * rRAW;
    R_reinf_thresh = RI * rRAA;
    
    B_step = R_step = 0;
    bchunks = rchunks = 0;
    
    a1 = (real) b_tot_reinf / B_maxchunks; /* Size of Blue blocks */
    a2 = (real) r_tot_reinf / R_maxchunks; /* Size of Red blocks */
    
    for (i = 0; i < MAXITER; i++) /* ~1/2 hour intervals */
    {
        if (B <= 0)
        {
            B = 0.0001; /* Don't divide by zero */
        }
        rc = R/B;       /* Force ratio */
        call_reinforcements(i); /* Call for reinforcements */
        reinforce(i);           /* Do the reinforcement */
        if (withdraw(i) == STOP) /* See if one side quits */
        {
            break;
        }
        attrit();               /* Blue and Red attrition */
    }
    exit(0);
}

