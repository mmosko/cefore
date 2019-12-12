/*
 * Copyright (c) 2016, National Institute of Information and Communications
 * Technology (NICT). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the NICT nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NICT AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE NICT OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * cef_face.h
 */

#ifndef __CEF_FACE_HEADER__
#define __CEF_FACE_HEADER__

/****************************************************************************************
 Include Files
 ****************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#ifndef CefC_Android
#include <sys/fcntl.h>
#else // CefC_Android
#include <fcntl.h>
#endif // CefC_Android
#include <unistd.h>
#include <sys/un.h>

#include <cefore/cef_hash.h>
#include <cefore/cef_define.h>

/****************************************************************************************
 Macros
 ****************************************************************************************/

/********** Reserved Face-IDs 				**********/

#define CefC_Face_Reserved			16

#define CefC_Faceid_Local			0
#define CefC_Faceid_ListenUdpv4		4
#define CefC_Faceid_ListenUdpv6		5
#define CefC_Faceid_ListenTcpv4		6
#define CefC_Faceid_ListenTcpv6		7

#define CefC_Faceid_ListenNdnv4		8
#define CefC_Faceid_ListenNdnv6		9

/********** FD for UNIX domain socket 		**********/
#ifndef CefC_Android
#define CefC_Local_Sock_Name		".cefore.sock"
#else // CefC_Android
/*
 * For Android Unix domain socket.
 * Android socket require special name.
 */
#define CefC_Local_Sock_Name		"\0org.app.cefore"
#define CefC_Local_Sock_Name_Len	14
#endif // CefC_Android

/********** Identifier to close Face 		**********/
#define CefC_Face_Close				"/CLOSE:Face"

/********** Neighbor Management				**********/
#define CefC_Max_RTT 				1000000		/* Maximum RTT (us) 					*/

/****************************************************************************************
 Structure Declarations
 ****************************************************************************************/

typedef struct {
	uint16_t		index;
	int				fd;
	unsigned char 	rcv_buff[CefC_Max_Length];
	uint16_t 		len;
	uint8_t 		local_f;
	uint8_t 		protocol;
	uint32_t 		seqnum;
} CefT_Face;

/********** Neighbor Management				**********/
typedef struct {
	uint16_t 			faceid;
	uint64_t 			rtt;
} CefT_Rtts;

/****************************************************************************************
 Global Variables
 ****************************************************************************************/



/****************************************************************************************
 Function Declarations
 ****************************************************************************************/

/*--------------------------------------------------------------------------------------
	Initialize the face module
----------------------------------------------------------------------------------------*/
int											/* Returns a negative value if it fails 	*/
cef_face_init (
	uint8_t 	node_type					/* Node Type (Router/Receiver....)			*/
);
/*--------------------------------------------------------------------------------------
	Creates the listening UDP socket with the specified port
----------------------------------------------------------------------------------------*/
int											/* Returns a negative value if it fails 	*/
cef_face_udp_listen_face_create (
	uint16_t 		port_num				/* Port Number that cefnetd listens			*/
);
/*--------------------------------------------------------------------------------------
	Creates the listening TCP socket with the specified port
----------------------------------------------------------------------------------------*/
int											/* Returns a negative value if it fails 	*/
cef_face_tcp_listen_face_create (
	uint16_t 		port_num				/* Port Number that cefnetd listens			*/
);
/*--------------------------------------------------------------------------------------
	Creates the listening UDP socket for NDN with the specified port
----------------------------------------------------------------------------------------*/
int											/* Returns a negative value if it fails 	*/
cef_face_ndn_listen_face_create (
	uint16_t 		port_num				/* Port Number that cefnetd listens			*/
);
/*--------------------------------------------------------------------------------------
	Accepts the TCP socket
----------------------------------------------------------------------------------------*/
int
cef_face_accept_connect (
	void
);
/*--------------------------------------------------------------------------------------
	Creates the local face that uses UNIX domain socket
----------------------------------------------------------------------------------------*/
int											/* Returns a negative value if it fails 	*/
cef_face_local_face_create (
	int sk_type
);
/*--------------------------------------------------------------------------------------
	Closes all faces
----------------------------------------------------------------------------------------*/
void
cef_face_all_face_close (
	void
);
/*--------------------------------------------------------------------------------------
	Checks the specified Face is active or not
----------------------------------------------------------------------------------------*/
int										/* Returns the value less than 1 if it fails 	*/
cef_face_check_active (
	int faceid								/* Face-ID									*/
);
/*--------------------------------------------------------------------------------------
	Obtains the Face structure from the specified Face-ID
----------------------------------------------------------------------------------------*/
uint32_t 
cef_face_get_seqnum_from_faceid (
	uint16_t 	faceid						/* Face-ID									*/
);
/*--------------------------------------------------------------------------------------
	Updates the listen faces with TCP
----------------------------------------------------------------------------------------*/
int											/* number of the listen face with TCP 		*/
cef_face_update_tcp_faces (
	struct pollfd* intcpfds, 
	uint16_t* intcpfaces, 
	uint8_t intcpfdc
);
/*--------------------------------------------------------------------------------------
	Converts the specified Face-ID into the corresponding file descriptor
----------------------------------------------------------------------------------------*/
int											/* the corresponding file descriptor		*/
cef_face_get_fd_from_faceid (
	uint16_t 		faceid					/* Face-ID									*/
);
/*--------------------------------------------------------------------------------------
	Looks up and creates the peer Face
----------------------------------------------------------------------------------------*/
int											/* Peer Face-ID 							*/
cef_face_lookup_peer_faceid (
	struct addrinfo* sas, 					/* sockaddr_storage structure				*/
	socklen_t sas_len,						/* length of sockaddr_storage				*/
	int protocol
);
/*--------------------------------------------------------------------------------------
	Looks up and creates the Face from the specified string of destination address
----------------------------------------------------------------------------------------*/
int											/* Face-ID									*/
cef_face_lookup_faceid_from_addrstr (
	const char* destination,				/* String of destination address 			*/
	const char* protocol					/* protoco (udp,tcp,local) 					*/
);
/*--------------------------------------------------------------------------------------
	Searches the specified Face
----------------------------------------------------------------------------------------*/
int											/* Face-ID									*/
cef_face_search_faceid (
	const char* destination, 				/* String of the destination address 		*/
	const char* protocol					/* protoco (udp,tcp,local) 					*/
);
/*--------------------------------------------------------------------------------------
	Sends a message via the specified Face
----------------------------------------------------------------------------------------*/
void
cef_face_frame_send_forced (
	uint16_t 		faceid, 				/* Face-ID indicating the destination 		*/
	unsigned char* 	msg, 					/* a message to send						*/
	size_t			msg_len					/* length of the message to send 			*/
);
/*--------------------------------------------------------------------------------------
	Obtains the Face structure from the specified Face-ID
----------------------------------------------------------------------------------------*/
CefT_Face* 									/* Face 									*/
cef_face_get_face_from_faceid (
	uint16_t 	faceid						/* Face-ID									*/
);
/*--------------------------------------------------------------------------------------
	Searches and creates the local Face-ID corresponding to the specified FD
----------------------------------------------------------------------------------------*/
int											/* the corresponding Face-ID				*/
cef_face_lookup_local_faceid (
	int fd									/* File descriptor							*/
);
/*--------------------------------------------------------------------------------------
	Closes the specified Face
----------------------------------------------------------------------------------------*/
int											/* Returns a negative value if it fails 	*/
cef_face_close (
	int faceid								/* Face-ID									*/
);
/*--------------------------------------------------------------------------------------
	Sends a Content Object via the specified Face
----------------------------------------------------------------------------------------*/
int											/* Returns a negative value if it fails 	*/
cef_face_object_send (
	uint16_t 		faceid, 				/* Face-ID indicating the destination 		*/
	unsigned char* 	msg, 					/* a message to send						*/
	size_t			msg_len,				/* length of the message to send 			*/
	unsigned char* 	payload, 				/* a message to send						*/
	size_t			payload_len,			/* length of the message to send 			*/
	uint32_t		chnk_num				/* Chunk Number 							*/
);
/*--------------------------------------------------------------------------------------
	Sends a Content Object if the specified is local Face
----------------------------------------------------------------------------------------*/
int											/* Returns a negative value if it fails 	*/
cef_face_object_send_iflocal (
	uint16_t 		faceid, 				/* Face-ID indicating the destination 		*/
	unsigned char* 	payload, 				/* a message to send						*/
	size_t			payload_len,			/* length of the message to send 			*/
	uint32_t		chnk_num				/* Chunk Number 							*/
);
/*--------------------------------------------------------------------------------------
	Sends a message if the specified is local Face with API Header
----------------------------------------------------------------------------------------*/
int											/* Returns a negative value if it fails 	*/
cef_face_apimsg_send_iflocal (
	uint16_t 		faceid, 				/* Face-ID indicating the destination 		*/
	void * 			api_hdr, 				/* a header to send						*/
	size_t			api_hdr_len,			/* length of the header to send 			*/
	void *		 	payload, 				/* a message to send						*/
	size_t			payload_len				/* length of the message to send 			*/
);
CefT_Hash_Handle*
cef_face_return_sock_table (
	void
);
/*--------------------------------------------------------------------------------------
	Checks whether the specified Face is local or not
----------------------------------------------------------------------------------------*/
int											/* local face is 1, no-local face is 0	 	*/
cef_face_is_local_face (
	uint16_t 		faceid 					/* Face-ID indicating the destination 		*/
);
/*--------------------------------------------------------------------------------------
	Obtains type of Face (local/UDP/TCP)
----------------------------------------------------------------------------------------*/
int											/* type of Face							 	*/
cef_face_type_get (
	uint16_t 		faceid 					/* Face-ID									*/
);
/*--------------------------------------------------------------------------------------
	Looks up the protocol type from the FD
----------------------------------------------------------------------------------------*/
int										/* Face-ID that is not used				*/
cef_face_get_protocol_from_fd (
	int fd
);

#endif // __CEF_FACE_HEADER__
