// Copyright (c) 2013, Cloudera, inc.
// All rights reserved.
#ifndef KUDU_RPC_RPC_CONTROLLER_H
#define KUDU_RPC_RPC_CONTROLLER_H

#include <glog/logging.h>
#include <tr1/memory>

#include "gutil/macros.h"
#include "util/locks.h"
#include "util/monotime.h"
#include "util/status.h"

namespace kudu { namespace rpc {

class OutboundCall;

// Controller for managing properties of a single RPC call, on the client side.
//
// An RpcController maps to exactly one call and is not thread-safe. The client
// may use this class prior to sending an RPC in order to set properties such
// as the call's timeout.
//
// After the call has been sent (e.g using Proxy::AsyncRequest()) the user
// may invoke methods on the RpcController object in order to probe the status
// of the call.
class RpcController {
 public:
  RpcController();
  ~RpcController();

  // Reset this controller so it may be used with another call.
  void Reset();

  // Return true if the call has finished.
  // A call is finished if the server has responded, or if the call
  // has timed out.
  bool finished() const;

  // Return the current status of a call.
  //
  // A call is "OK" status until it finishes, at which point it may
  // either remain in "OK" status (if the call was successful), or
  // change to an error status. Error status indicates that there was
  // some RPC-layer issue with making the call, for example, one of:
  //
  // * failed to establish a connection to the server
  // * the server was too busy to handle the request
  // * the server was unable to interpret the request (eg due to a version
  //   mismatch)
  // * a network error occurred which caused the connection to be torn
  //   down
  // * the call timed out
  Status status() const;

  // Set the timeout for the call to be made with this RPC controller.
  //
  // The configured timeout applies to the entire time period between
  // the AsyncRequest() method call and getting a response. For example,
  // if it takes too long to establish a connection to the remote host,
  // or to DNS-resolve the remote host, those will be accounted as part
  // of the timeout period.
  //
  // Timeouts must be set prior to making the request -- the timeout may
  // not currently be adjusted for an already-sent call.
  //
  // Setting the timeout to 0 will result in a call which never times out
  // (not recommended!)
  void set_timeout(const MonoDelta &timeout);

  // Return the configured timeout.
  const MonoDelta &timeout() const;

 private:
  friend class OutboundCall;
  friend class Proxy;

  MonoDelta timeout_;

  mutable simple_spinlock lock_;

  // Once the call is sent, it is tracked here.
  std::tr1::shared_ptr<OutboundCall> call_;

  DISALLOW_COPY_AND_ASSIGN(RpcController);
};

} // namespace rpc
} // namespace kudu
#endif
