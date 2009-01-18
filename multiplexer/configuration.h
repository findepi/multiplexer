#ifndef MULTIPLEXER_CONFIG_H
#define MULTIPLEXER_CONFIG_H

namespace multiplexer {

    static const unsigned int DEFAULT_INCOMING_QUEUE_MAX_SIZE = 1024;
    static const unsigned int AUTO_RECONNECT_TIME = 3;
    static const float DEFAULT_TIMEOUT = 10.0;
    static const float DEFAULT_READ_TIMEOUT = -1; // "< 0" means inifinity
    static const unsigned int MAX_MESSAGE_SIZE = 128 * 1024 * 1024;

    static const float HEARTBIT_INTERVAL = 3.0;
    static const float NO_HEARTBIT_SO_PREPARE_DROP_INTERVAL = 5 * 60;
    static const float NO_HEARTBIT_SO_REALLY_DROP_INTERVAL = 10;

}; // namespace multiplexer

#endif
