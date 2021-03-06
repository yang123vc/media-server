#ifndef _sip_uas_transaction_h_
#define _sip_uas_transaction_h_

#include "sip-uas.h"
#include "sip-timer.h"
#include "sip-header.h"
#include "sip-message.h"
#include "sip-transport.h"
#include "sys/atomic.h"
#include "sys/locker.h"
#include "list.h"

#define UDP_PACKET_SIZE 1440

enum
{
	SIP_UAS_TRANSACTION_INIT,
	SIP_UAS_TRANSACTION_TRYING,
	SIP_UAS_TRANSACTION_PROCEEDING,
	SIP_UAS_TRANSACTION_COMPLETED,
	SIP_UAS_TRANSACTION_CONFIRMED,
	SIP_UAS_TRANSACTION_TERMINATED,
};

struct sip_uas_transaction_t
{
	struct list_head link;
	locker_t locker;
	int32_t ref;

	void* session; // user-defined session
	uint8_t data[UDP_PACKET_SIZE];
	int size;
	int retransmission; // default 0
	int reliable; // last transport->reliable, 0-udp, 1-tcp

	int32_t status;
	int retries;
	int t2; // 64*T1-invite, 4s-noninvite
	void* timerg; // T1 --> (2*T1, T2) retransmission timer, same as Timer-A
	void* timerh; // 64*T1, timeout, same as Timer-B
	void* timerij; // Timer-I: T4, comfirmed -> terminated, Timer-J: 64*T1, completed -> terminated

	struct sip_uas_t* uas;
	struct sip_uas_handler_t* handler;
	void* param;

	// valid only in [sip_uas_input, sip_uas_reply]
	// create in sip_uas_input, destroy in sip_uas_reply
	const struct sip_message_t* req;
	struct sip_message_t* reply; // for set reply sip header
};

struct sip_uas_transaction_t* sip_uas_transaction_create(struct sip_uas_t* uas, const struct sip_message_t* msg);
int sip_uas_transaction_release(struct sip_uas_transaction_t* t);
int sip_uas_transaction_addref(struct sip_uas_transaction_t* t);

int sip_uas_transaction_dosend(struct sip_uas_transaction_t* t);

struct sip_uas_transaction_t* sip_uas_find_transaction(struct sip_uas_t* uas, const struct sip_message_t* req, int matchmethod);
int sip_uas_transaction_handler(struct sip_uas_transaction_t* t, struct sip_dialog_t* dialog, const struct sip_message_t* req);
int sip_uas_onoptions(struct sip_uas_transaction_t* t, const struct sip_message_t* req);
int sip_uas_onregister(struct sip_uas_transaction_t* t, const struct sip_message_t* req);
int sip_uas_oncancel(struct sip_uas_transaction_t* t, struct sip_dialog_t* dialog, const struct sip_message_t* req);
int sip_uas_onbye(struct sip_uas_transaction_t* t, struct sip_dialog_t* dialog, const struct sip_message_t* req);

int sip_uas_transaction_invite_input(struct sip_uas_transaction_t* t, struct sip_dialog_t* dialog, const struct sip_message_t* req);
int sip_uas_transaction_invite_reply(struct sip_uas_transaction_t* t, int code, const void* data, int bytes);
int sip_uas_transaction_noninvite_input(struct sip_uas_transaction_t* t, struct sip_dialog_t* dialog, const struct sip_message_t* req);
int sip_uas_transaction_noninvite_reply(struct sip_uas_transaction_t* t, int code, const void* data, int bytes);

int sip_uas_del_transaction(struct sip_uas_t* uas, struct sip_uas_transaction_t* t);
int sip_uas_add_transaction(struct sip_uas_t* uas, struct sip_uas_transaction_t* t);
int sip_uas_add_dialog(struct sip_uas_t* uas, struct sip_dialog_t* dialog);
int sip_uas_del_dialog(struct sip_uas_t* uas, struct sip_dialog_t* dialog);
void* sip_uas_start_timer(struct sip_uas_t* uas, int timeout, sip_timer_handle handler, void* usrptr);
void sip_uas_stop_timer(struct sip_uas_t* uas, void* id);
const char* sip_reason_phrase(int code);

#endif /* !_sip_uas_transaction_h_ */
