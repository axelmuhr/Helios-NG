// Request codes for inclink

#ifndef _requests_h_
#define _requests_h_

enum RequestType {
    // requests
    inclink_dflag,
    inclink_dolink,
    inclink_fulllink,
    inclink_kflag,
    inclink_ldflag,        // imitate ld flag
    inclink_linkfile,
    inclink_modfile,       // the following object file is modified
    inclink_quit,
    inclink_Sflag,
    inclink_sflag,
    inclink_timer,
    inclink_tinclink,
    inclink_Xflag,
    inclink_xflag,
    inclink_maxreq
};

char* requestName[] = {
    "inclink_debug",
    "inclink_dolink",
    "inclink_fulllink",
    "inclink_kflag",
    "inclink_ldflag",        // imitate ld flag
    "inclink_linkfile",
    "inclink_modfile",       // the following object file is modified
    "inclink_quit",
    "inclink_Sflag",
    "inclink_sflag",
    "inclink_timer",
    "inclink_tinclink",
    "inclink_Xflag",
    "inclink_xflag",
    0
};

#endif
