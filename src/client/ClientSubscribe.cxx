/*
 * Copyright 2003-2016 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"
#include "ClientInternal.hxx"
#include "Idle.hxx"

#include <assert.h>

Client::SubscribeResult
Client::Subscribe(const char *channel)
{
	assert(channel != nullptr);

	if (!client_message_valid_channel_name(channel))
		return Client::SubscribeResult::INVALID;

	if (num_subscriptions >= CLIENT_MAX_SUBSCRIPTIONS)
		return Client::SubscribeResult::FULL;

	auto r = subscriptions.insert(channel);
	if (!r.second)
		return Client::SubscribeResult::ALREADY;

	++num_subscriptions;

	idle_add(IDLE_SUBSCRIPTION);

	return Client::SubscribeResult::OK;
}

bool
Client::Unsubscribe(const char *channel)
{
	const auto i = subscriptions.find(channel);
	if (i == subscriptions.end())
		return false;

	assert(num_subscriptions > 0);

	subscriptions.erase(i);
	--num_subscriptions;

	idle_add(IDLE_SUBSCRIPTION);

	assert((num_subscriptions == 0) ==
	       subscriptions.empty());

	return true;
}

void
Client::UnsubscribeAll()
{
	subscriptions.clear();
	num_subscriptions = 0;
}

bool
Client::PushMessage(const ClientMessage &msg)
{
	if (messages.size() >= CLIENT_MAX_MESSAGES ||
	    !IsSubscribed(msg.GetChannel()))
		return false;

	if (messages.empty())
		IdleAdd(IDLE_MESSAGE);

	messages.push_back(msg);
	return true;
}
