#pragma once

#include <amqp.h>

#include <QString>

namespace amqp
{


void fail_if(bool predicate, const std::string &message);
void fail_if_error(const amqp_rpc_reply_t &reply, const std::string &message);

amqp_connection_state_t create_connection();
void destroy_connection(amqp_connection_state_t conn);

void* connect(const amqp_connection_state_t conn, const QString &hostname, uint16_t port);
void disconnect(const amqp_connection_state_t conn);

const amqp_channel_t *create_channel(const amqp_connection_state_t &conn, unsigned short channel);
void destroy_channel(const amqp_connection_state_t conn, const amqp_channel_t *chptr);


}
