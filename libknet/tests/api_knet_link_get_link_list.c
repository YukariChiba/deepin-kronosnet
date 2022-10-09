/*
 * Copyright (C) 2016-2021 Red Hat, Inc.  All rights reserved.
 *
 * Authors: Fabio M. Di Nitto <fabbione@kronosnet.org>
 *
 * This software licensed under GPL-2.0+
 */

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libknet.h"

#include "internals.h"
#include "link.h"
#include "netutils.h"
#include "test-common.h"

static void test(void)
{
	knet_handle_t knet_h;
	int logfds[2];
	uint8_t link_ids[KNET_MAX_LINK];
	size_t link_ids_entries = 0;
	struct sockaddr_storage lo;

	memset(&link_ids, 1, sizeof(link_ids));

	printf("Test knet_link_get_link_list incorrect knet_h\n");

	if ((!knet_link_get_link_list(NULL, 1, link_ids, &link_ids_entries)) || (errno != EINVAL)) {
		printf("knet_link_get_link_list accepted invalid knet_h or returned incorrect error: %s\n", strerror(errno));
		exit(FAIL);
	}

	setup_logpipes(logfds);

	knet_h = knet_handle_start(logfds, KNET_LOG_DEBUG);

	printf("Test knet_link_get_link_list with unconfigured host_id\n");

	if ((!knet_link_get_link_list(knet_h, 1, link_ids, &link_ids_entries)) || (errno != EINVAL)) {
		printf("knet_link_get_link_list accepted invalid host_id or returned incorrect error: %s\n", strerror(errno));
		knet_handle_free(knet_h);
		flush_logs(logfds[0], stdout);
		close_logpipes(logfds);
		exit(FAIL);
	}

	flush_logs(logfds[0], stdout);

	printf("Test knet_link_get_link_list with incorrect link_id\n");

	if (knet_host_add(knet_h, 1) < 0) {
		printf("Unable to add host_id 1: %s\n", strerror(errno));
		knet_handle_free(knet_h);
		flush_logs(logfds[0], stdout);
		close_logpipes(logfds);
		exit(FAIL);
	}

	if ((!knet_link_get_link_list(knet_h, 1, NULL, &link_ids_entries)) || (errno != EINVAL)) {
		printf("knet_link_get_link_list accepted invalid link_ids or returned incorrect error: %s\n", strerror(errno));
		knet_host_remove(knet_h, 1);
		knet_handle_free(knet_h);
		flush_logs(logfds[0], stdout);
		close_logpipes(logfds);
		exit(FAIL);
	}

	flush_logs(logfds[0], stdout);

	printf("Test knet_link_get_link_list with incorrect link_ids_entries\n");

	if ((!knet_link_get_link_list(knet_h, 1, link_ids, NULL)) || (errno != EINVAL)) {
		printf("knet_link_get_link_list accepted invalid link_ids_entries or returned incorrect error: %s\n", strerror(errno));
		knet_host_remove(knet_h, 1);
		knet_handle_free(knet_h);
		flush_logs(logfds[0], stdout);
		close_logpipes(logfds);
		exit(FAIL);
	}

	flush_logs(logfds[0], stdout);

	printf("Test knet_link_get_link_list with no links\n");

	if (knet_link_get_link_list(knet_h, 1, link_ids, &link_ids_entries) < 0) {
		printf("knet_link_get_link_list failed: %s\n", strerror(errno));
		knet_host_remove(knet_h, 1);
		knet_handle_free(knet_h);
		flush_logs(logfds[0], stdout);
		close_logpipes(logfds);
		exit(FAIL);
	}

	if (link_ids_entries != 0) {
		printf("knet_link_get_link_list returned incorrect number of links");
		knet_host_remove(knet_h, 1);
		knet_handle_free(knet_h);
		flush_logs(logfds[0], stdout);
		close_logpipes(logfds);
		exit(FAIL);

	}

	flush_logs(logfds[0], stdout);

	printf("Test knet_link_get_link_list with 1 link\n");

	if (_knet_link_set_config(knet_h, 1, 0, KNET_TRANSPORT_UDP, 0, AF_INET, 0, &lo) < 0) {
		printf("Unable to configure link: %s\n", strerror(errno));
		knet_host_remove(knet_h, 1);
		knet_handle_free(knet_h);
		flush_logs(logfds[0], stdout);
		close_logpipes(logfds);
		exit(FAIL);
	}

	if (knet_link_get_link_list(knet_h, 1, link_ids, &link_ids_entries) < 0) {
		printf("knet_link_get_link_list failed: %s\n", strerror(errno));
		knet_link_clear_config(knet_h, 1, 0);
		knet_host_remove(knet_h, 1);
		knet_handle_free(knet_h);
		flush_logs(logfds[0], stdout);
		close_logpipes(logfds);
		exit(FAIL);
	}

	if ((link_ids_entries != 1) || (link_ids[0] != 0)) {
		printf("knet_link_get_link_list returned incorrect values");
		knet_link_clear_config(knet_h, 1, 0);
		knet_host_remove(knet_h, 1);
		knet_handle_free(knet_h);
		flush_logs(logfds[0], stdout);
		close_logpipes(logfds);
		exit(FAIL);
	}

	flush_logs(logfds[0], stdout);

	knet_link_clear_config(knet_h, 1, 0);
	knet_host_remove(knet_h, 1);
	knet_handle_free(knet_h);
	flush_logs(logfds[0], stdout);
	close_logpipes(logfds);
}

int main(int argc, char *argv[])
{
	test();

	return PASS;
}
