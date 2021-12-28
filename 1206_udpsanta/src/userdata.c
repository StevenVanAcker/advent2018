#include "config.h"
#include "userdata.h"

AllDataBlob AllData;
UserMsg *globMsgStore = (UserMsg *)&(AllData.embedMsgStore);
UserKey *globKeyStore = (UserKey *)&(AllData.embedKeyStore);
uint32_t *globMsgIdList = (uint32_t *)&(AllData.embedMsgIdList);


void read_userdata(char *userlistfile, char *datadir) {
	char *userlist = read_file_as_string(userlistfile);
	char *p = userlist, *u = userlist;
	int usercounter = 0;
	int msgcounter = 0;

	if(!userlist) {
		ERROR("Couldn't read userlist from %s\n", userlistfile);
	}

	while(p) {
		p = strchr(p, '\n');
		if(p) { *p = '\0'; p++; }
		if(strlen(u) > 0) {

			if(usercounter < MAXUSERS) {
				char filename[1024];
				char *pubkey, *key, *iv;

				snprintf(filename, sizeof(filename), "%s/keys/%s.pub", datadir, u);
				pubkey = read_file_as_string(filename);
				snprintf(filename, sizeof(filename), "%s/keys/%s.key", datadir, u);
				key = read_file_as_string(filename);
				snprintf(filename, sizeof(filename), "%s/keys/%s.iv", datadir, u);
				iv = read_file_as_string(filename);

				if(!(pubkey && key && iv)) {
					ERROR("Could not find all keyfiles for user %s in %s. Make sure these files exist: %s/keys/%s.{pub,key,iv}\n", u, datadir, datadir, u);
				}

				strncpy(globKeyStore[usercounter].name, u, NAMESIZE);
				globKeyStore[usercounter].pubkey = pubkey;
				pubkey = NULL; /* don't free this */
				strncpy(globKeyStore[usercounter].key, key, KEYSIZE);
				strncpy(globKeyStore[usercounter].iv, iv, IVSIZE);

				myfree(key);
				myfree(iv);

				// read messages
				for(int i = 0; i < MSGSLISTSIZE; i++) {
					char *from;
					char *id;
					int mid;
					char *msg;

					snprintf(filename, sizeof(filename), "%s/msgs/%s.%d.id", datadir, u, i);
					id = read_file_as_string(filename);
					snprintf(filename, sizeof(filename), "%s/msgs/%s.%d.msg", datadir, u, i);
					msg = read_file_as_string(filename);
					snprintf(filename, sizeof(filename), "%s/msgs/%s.%d.from", datadir, u, i);
					from = read_file_as_string(filename);

					if(!(from && id && msg)) {
						break;
					}

					strncpy(globMsgStore[msgcounter].name, u, NAMESIZE);
					strncpy(globMsgStore[msgcounter].from, from, NAMESIZE);
					strncpy(globMsgStore[msgcounter].msg, msg, MSGLEN);
					globMsgStore[msgcounter].id = atoi(id);

					myfree(id);
					myfree(msg);
					myfree(from);

					msgcounter++;
				}
				usercounter++;
			} else {
				ERROR("Trying to read more than %d users.\n", MAXUSERS);
			}
		}
		u = p;
	}

	myfree(userlist);
}

void dump_userdata() {
	int emptycounter = 0;
	for(int i = 0; i < MAXUSERS * MSGSLISTSIZE; i++) {
		UserMsg *x = &globMsgStore[i];
		if(strlen(x->name) == 0 && strlen(x->from) == 0 && x->id == 0 && strlen(x->msg) == 0) {
			emptycounter++;
		} else {
			DEBUG("MSG %d name = %s\n", i, x->name);
			DEBUG("MSG %d from = %s\n", i, x->from);
			DEBUG("MSG %d id = %d\n", i, x->id);
			DEBUG("MSG %d msg =\n%s\n", i, x->msg);
		}
	}
	DEBUG("Found %d empty UserMsg entries\n", emptycounter);

	emptycounter = 0;
	for(int i = 0; i < MSGSLISTSIZE; i++) {
		if(globMsgIdList[i] != -1) {
			DEBUG("MSGID %d = %d\n", i, globMsgIdList[i]);
		} else {
			emptycounter++;
		}
	}
	DEBUG("Found %d empty MsgID entries\n", emptycounter);

	for(int i = 0; i < MAXUSERS; i++) {
		UserKey *x = &globKeyStore[i];
		if(strlen(x->name) == 0 && x->pubkey == NULL && strlen(x->key) == 0 && strlen(x->iv) == 0) {
			emptycounter++;
		} else {
			DEBUG("KEY %d name = %s\n", i, x->name);
			DEBUG("KEY %d key = %s\n", i, x->key);
			DEBUG("KEY %d iv = %s\n", i, x->iv);
			DEBUG("KEY %d pubkey =\n%s\n", i, x->pubkey);
		}
	}
	DEBUG("Found %d empty UserKey entries\n", emptycounter);
}

void copy_msgids(char *username) {
	// first, fill the buffer with -1
	for(int i = 0; i < MSGSLISTSIZE; i++) globMsgIdList[i] = -1;
	
	// now, let's find some messages...
	int msgidcounter = 0;

	for(int i = 0; i < MAXUSERS * MSGSLISTSIZE; i++) {
		UserMsg *x = &globMsgStore[i];
		if(strlen(x->name) == 0 && strlen(x->from) == 0 && x->id == 0 && strlen(x->msg) == 0) {
			return;
		} else {
			if(msgidcounter < MSGSLISTSIZE && !strcmp(x->name, username)) {
				globMsgIdList[msgidcounter] = x->id;
				msgidcounter++;
			}
		}
	}
}

UserMsg *get_user_msg(char *username, uint32_t id) {
	for(int i = 0; i < MAXUSERS * MSGSLISTSIZE; i++) {
		UserMsg *x = &globMsgStore[i];
		if(id == x->id && !strcmp(username, x->name)) return x;
	}

	return NULL;
}

UserKey *get_user_key(char *username) {
	for(int i = 0; i < MAXUSERS; i++) {
		UserKey *x = &globKeyStore[i];
		if(!strcmp(username, x->name)) return x;
	}

	return NULL;
}






