/*
 * Replies for inclink clients from the server
 */

#ifndef _replies_h_
#define _replies_h_

enum ReplyType {
    inclink_ack,      // no errors

                      // other replies are followed by two messages:
                      //     1) length of following error message
                      //     2) error message

    inclink_debug,    // tracing
    inclink_warning,  // warning, but inclink completed
    inclink_error,    // inclink not completed, but not exited
    inclink_fatal,    // inclink exited due to user error
    inclink_panic     // inclink exited due to internal error
};

#endif
