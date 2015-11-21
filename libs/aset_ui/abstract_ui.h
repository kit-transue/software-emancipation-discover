// copyright 2007, Coverity, Inc.
//
// Generic interface for Discover UI operations.
// Implementations might include {motif, galaxy, Java, TCL/Express, none}

class discoverUI {
 public:
    /**
     * Notify that queries are operating in a given closure mode.
     *
     * @param enabled  Whether closure mode is enabled.
     **/
    virtual ~discoverUI() {}
    virtual void setClosureState(bool enabled) = 0;
    virtual void refreshAfterModuleDelete() = 0;
};

discoverUI *theUI;
