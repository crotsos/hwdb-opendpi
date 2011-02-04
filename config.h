/*
 * The Homework Database
 *
 * Authors:
 *    Oliver Sharma and Joe Sventek
 *     {oliver, joe}@dcs.gla.ac.uk
 *
 * (c) 2009. All rights reserved.
 */
#ifndef HWDB_CONFIG_H
#define HWDB_CONFIG_H


/* Server */
#define HWDB_SERVER_ADDR "localhost"
#define HWDB_SERVER_PORT 987
#define HWDB_SNAPSHOT_PORT 988

#define SOCK_RECV_BUF_LEN 65535

#define RTAB_MSG_MAX_LENGTH 100

/* Index */
#define TT_BUCKETS 100

/* Saved-queries hashtable */
#define HT_QUERYTAB_BUCKETS 100

/* Multi-threading */
#define HWDB_PUBLISH_IN_BACKGROUND
#define NUM_THREADS 2


/* Debug settings */

#endif
