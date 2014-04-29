from __future__ import print_function


# Constants
YES = 1
NO = 0
STOP = 1
GO = 0

# XXX MAXITER Should be a class function / configuration thing
MAXITER = 200000

class model():
    """
    Model, derived from the model in appendix A of R3995-RC.
    Based on the C implementation in this git repo.
    """
    
    # Bunch of default configuration options
    RI = 1000         # Initial number of Red troops 
    BI = 500          # Initial number of Blue troops 

    c1 = 1.0 / 2048.0 # Defenders attrition rate  
    c2 = 1.0 / 512.0  # Attackers " " 

    rBA = 4           # Blue calls for reinforcements at 1:4 
    rRA = 2.5         # Read calls for reinforcements at 2.5:1 

    rBW = 10          # Blue withdrawls at 1:10 
    rRW = 1.5         # Red withdrawls at 1.5:1 

    rBAA = 0.80       # Blue calls for more if attrition is high 
    rRAA = 0.80       # Red " 

    rBAW = 0.70       # Blue withdrawls if down to this 
    rRAW = 0.70       # Red " 

    B_delay = 70       # Arrival time delay of reinforcements 
    R_delay = 70       # Arrival time delay of reinforcements 

    B_maxchunks = 5    # How many chunks may they use? 
    R_maxchunks = 5    # How many chunks may they use? 

    b_tot_reinf = 1500 # Total reinforcements 
    r_tot_reinf = 1500
    
    def __init__(self, **kwargs):
        '''
        
        '''
        # Values used when running the simulation
        self.a1 = 0.0 # Reinforcement chunk size for Blue
        self.a2 = 0.0 # "             "      "    "  Red

        self.bchunks = 0.0
        self.rchunks = 0.0 # How many chunks have Blue and Red ordered?

        self.B_reinf = 0.0
        self.R_reinf = 0.0  # Total reinforcements tossed in

        self.B_step = 0
        self.R_step = 0 # Arrival time step of reinforcements
        
        self.B_ordered = 0
        self.R_ordered = 0 # How many reinforcements are ordered

        self.B_with_thresh = 0.0
        self.R_with_thresh = 0.0
        self.B_reinf_thresh = 0.0
        self.R_reinf_thresh = 0.0

        self.rc = 0.0 # Force ratio: Red/Blue
        self.R = 0 # Current number of Red/Blue troops
        self.B = 0
        
        # XXX Placeholder for later tweaking parameters previously defined as class members
        
        # Now prep the model
        self.prep_model()
        
    def prep_model(self):
        '''
        Set up a bunch of ratios required in order to run the model
        '''
        self.B = self.BI
        self.R = self.RI
        
        self.B_with_thresh = self.BI * self.rBAW
        self.B_reinf_thresh = self.BI * self.rBAA
        
        self.R_with_thresh = self.RI * self.rRAW
        self.R_reinf_thresh = self.RI * self.rRAA
        
        self.a1 = self.b_tot_reinf / self.B_maxchunks # Size of Blue blocks
        self.a2 = self.r_tot_reinf / self.R_maxchunks # Size of Red blocks
        
        
    def run_model(self, **kwargs):
        '''
        XXX Eventually this should take some interesting arguments
        '''
        for i in xrange(0, MAXITER):
            if self.B <= 0:
                self.B = 0.0001             # Don't divide by zero
            self.rc = self.R / self.B       # Set force ratio
            self.call_reinforcements(i)     # Call for reinforcements
            self.reinforce(i)               # Do the reinforcement
            if self.withdraw(i) == STOP:    # See if one side quits
                break
            self.attrit()                   # Blue and Red attrition
        return
    
    def call_reinforcements(self, i):
        '''
        See if we need to call for reinforcements
        
        i   -   The current time step
        '''
        # Blue reinforcement based on attrition
        if (self.B_ordered == 0 and self.bchunks < self.B_maxchunks and self.B < self.B_reinf_thresh):
            self.B_step = i + self.B_delay
            self.B_ordered = self.a1
            self.bchunks = self.bchunks + 1
            print("Blue calls for reinforcements /A at %s." % str(i))
            
        # Blue reinforcements based on force ratio
        if (self.B_ordered == 0 and self.bchunks < self.B_maxchunks and self.rc > self.rBA):
            self.B_step = i + self.B_delay
            self.B_ordered = self.a1
            self.bchunks = self.bchunks + 1
            print("Blue calls for reinforcements /FR at %s." % str(i))

        # Red reinforcement based on attrition 
        if (self.R_ordered == 0 and self.rchunks < self.R_maxchunks and self.B < self.R_reinf_thresh):
            self.R_step = i + self.R_delay
            self.R_ordered = self.a2
            self.rchunks = self.rchunks + 1
            print("Red calls for reinforcements /A at %s." % str(i))
            
        #Red reinforcements based on force ratio
        if (self.R_ordered == 0 and self.rchunks < self.R_maxchunks and self.rc < self.rRA):
            self.R_step = i + self.R_delay
            self.R_ordered = self.a2
            self.rchunks = self.rchunks + 1 
            print("Red calls for reinforcements /FR at %s." % str(i))
        
        return

    def reinforce(self, i):
        '''
        Do the reinforcement
        
        i   -   The current time step
        '''
        if (self.B_ordered > 0 and self.B_step <= i):
            self.B = self.B + self.B_ordered
            self.B_reinf = self.B_reinf + self.B_ordered
            self.B_ordered = 0
            print("Blue reinforcements arrive at %s." % str(i))
        if (self.R_ordered > 0 and self.R_step <= i):
            self.R = self.R + self.R_ordered
            self.R_reinf = self.R_reinf + self.R_ordered
            self.R_ordered = 0
            print("Red reinfocements arrive at %s." % str(i))
        return
        
    def withdraw(self, i):
        '''
        Check for withdrawal based on force ratio or attrition
        
        i   -   The current time step
        
        XXX What if BOTH of the sides would withdraw in the
        same timestep?  The original logic defaults to the Blue
        team withdrawling first, but this is not a very good
        model since it has a built in bias instead of a third
        outcome.
        '''
        if (self.B < self.B_with_thresh or self.rc > self.rBW):
            # XXX No status indicated as to WHY the withdrawl is occuring 
            print("Blue withdrawls at %s." % str(i))
            return STOP
        if (self.R < self.R_with_thresh or self.rc < self.rRW):
            # XXX No status indicated as to WHY the withdrawl is occuring 
            print("Red withdrawls at %s." % str(i))
            return STOP
        return GO
        
    def attrit(self):
        '''
        Throw stones at frogs in sport
        
        The use of b as a temporary store of the global
        value B is not entirely clear here, as it is derived from
        the formulae used to define the model in the R3995 paper
        '''
        b = self.B
        t = self.c1 * self.R
        self.B = self.B - t
        t = self.c2 * b
        self.R = self.R - t
        return