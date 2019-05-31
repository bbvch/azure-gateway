/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "utility.h"

#include <amqp_tcp_socket.h>

#include <stdexcept>
#include <map>


namespace amqp
{
namespace
{


const std::map<amqp_method_number_t, std::string> MethodNames {
  {AMQP_CONNECTION_CLOSE_METHOD, "connection"},
  {AMQP_CHANNEL_CLOSE_METHOD, "channel"},
};

std::string serverExceptionMessage(const amqp_method_t& error)
{
  const auto id = error.id;
  const auto idx = MethodNames.find(id);
  if (idx == MethodNames.end()) {
    std::string fe = "unknown server error, method id " + std::to_string(id);
    return fe;
  } else {
    amqp_connection_close_t *m = static_cast<amqp_connection_close_t*>(error.decoded);
    std::string em{static_cast<const char *>(m->reply_text.bytes), m->reply_text.len};
    std::string fe = "server " + idx->second + " error " + std::to_string(m->reply_code) +  "; " + em;
    return fe;
  }
}


}


void fail_if(bool predicate, const std::string &message)
{
  if (predicate) {
    throw std::runtime_error(message);
  }
}

void fail_if_error(const amqp_rpc_reply_t& reply, const std::string &message)
{
  if (reply.reply_type == AMQP_RESPONSE_SERVER_EXCEPTION) {
    const amqp_method_t& mr = reply.reply;
    const std::string fe = message + ": " + serverExceptionMessage(mr);
    throw std::runtime_error(fe);
  }
  fail_if(reply.reply_type != AMQP_RESPONSE_NORMAL, message);
}

amqp_connection_state_t create_connection()
{
  const auto conn = amqp_new_connection();
  fail_if(!conn, "can not create connection");
  return conn;
}

void destroy_connection(amqp_connection_state_t conn)
{
  amqp_destroy_connection(conn);
}

void* connect(const amqp_connection_state_t conn, const QString &hostname, uint16_t port)
{
  const auto socket = amqp_tcp_socket_new(conn);
  fail_if(!socket, "can not create TCP socket");

  const auto status = amqp_socket_open(socket, hostname.toStdString().c_str(), port);
  fail_if(status, "can not open TCP socket");

  const auto res = amqp_login(conn, "/", 0, 0x20000, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
  fail_if_error(res, "can not log into amqp server");

  return nullptr;
}

void disconnect(const amqp_connection_state_t conn)
{
  amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
}

const amqp_channel_t* create_channel(const amqp_connection_state_t &conn, unsigned short channel)
{
  amqp_channel_t *chptr = new amqp_channel_t;
  *chptr = channel;
  amqp_channel_open(conn, *chptr);
  const auto res = amqp_get_rpc_reply(conn);
  fail_if_error(res, "can not open channel");
  return chptr;
}

void destroy_channel(const amqp_connection_state_t conn, const amqp_channel_t *chptr)
{
  amqp_channel_close(conn, *chptr, AMQP_REPLY_SUCCESS);
  delete chptr;
}


}
