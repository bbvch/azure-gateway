#include "amqp_Client.h"

#include "logger.h"


namespace amqp
{
namespace
{

const QString hostname = "localhost";
const uint16_t port = 5672;
const amqp_channel_t channel_number = 1;

}


Client::Client(const QString &queuename, QObject *parent) :
    QObject{parent},
    conn{create_connection(), &destroy_connection},
    open_connection{::amqp::connect(conn.get(), hostname, port), [this](void*){{::amqp::disconnect(conn.get());}}},
    channel{create_channel(conn.get(), channel_number), [this](const amqp_channel_t *channel){destroy_channel(conn.get(), channel);}}
{
    amqp_basic_consume(conn.get(), *channel.get(), amqp_cstring_bytes(queuename.toStdString().c_str()), amqp_empty_bytes, 0, 0, 1, amqp_empty_table);
    fail_if_error(amqp_get_rpc_reply(conn.get()), "can not start consuming from queue " + queuename.toStdString());
}

QString to_string(const amqp_bytes_t &value)
{
    return QString::fromLocal8Bit(static_cast<const char*>(value.bytes), value.len);
}

amqp_envelope_t* consume_message(amqp_envelope_t *envelope, const amqp_connection_state_t &conn)
{
    struct timeval timeout = {0, 0};

    const auto res = amqp_consume_message(conn, envelope, &timeout, 0);

    return (AMQP_RESPONSE_NORMAL == res.reply_type) ? envelope : nullptr;
}

QStringMap readHeaders(const amqp_basic_properties_t &properties)
{
    QStringMap result{};

    if ((properties._flags & AMQP_BASIC_HEADERS_FLAG) != 0) {
        const auto headers = properties.headers;

        for (int i = 0; i < headers.num_entries; i++) {
            const auto &entry = headers.entries[i];
            const auto key = to_string(entry.key);
            switch (entry.value.kind) {
            case AMQP_FIELD_KIND_UTF8: {
                const auto value = to_string(entry.value.value.bytes);
                result[key] = value;
                break;
            }
            default: {
                qCWarning(logger) << "received header" << key << "with unhandled type" << QChar{entry.value.kind};
                break;
            }
            }
        }
    }

    return result;
}

void Client::tick()
{
    if (unansweredMessages.size() > 0) {
        return;
    }

    amqp_maybe_release_buffers(conn.get());

    const std::unique_ptr<amqp_envelope_t> envelope_memory{new amqp_envelope_t};
    std::unique_ptr<amqp_envelope_t, decltype(&amqp_destroy_envelope)> message{consume_message(envelope_memory.get(), conn.get()), &amqp_destroy_envelope};

    if (!message) {
        return;
    }

    const auto body = to_string(message->message.body);
    const auto content_type = to_string(message->message.properties.content_type);
    const auto content_encoding = to_string(message->message.properties.content_encoding);
    const auto headers = readHeaders(message->message.properties);

    qCDebug(logger) << "received message:" << message->delivery_tag;
    unansweredMessages.push_back(message->delivery_tag);

    received(body, content_type, content_encoding, headers);
}

void Client::ackLastMessage()
{
    if (unansweredMessages.empty()) {
        qCWarning(logger) << "can not ack message with no unanswerded messages";
        return;
    }

    const auto tag = unansweredMessages.first();

    const int ack_res = amqp_basic_ack(conn.get(), *channel.get(), tag, 0);
    if (ack_res == 0) {
        qCDebug(logger) << "acknowledged message:" << tag;
        unansweredMessages.pop_front();
        acked();
    } else {
        qFatal("can not ack message");
    }
}


}
