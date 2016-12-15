#ifndef MDNS_H
#define MDNS_H

#define MDNS_TYPE_A     0x0001
#define MDNS_TYPE_PTR   0x000C
#define MDNS_TYPE_HINFO 0x000D
#define MDNS_TYPE_TXT   0x0010
#define MDNS_TYPE_AAAA  0x001C
#define MDNS_TYPE_SRV   0x0021

#define MDNS_TARGET_PORT 5353
#define MDNS_SOURCE_PORT 5353
#define MDNS_TTL 255

#define MAX_PACKET_SIZE 2048   // Make this as big as memory limitations allow.
#define MAX_MDNS_NAME_LEN 256  // The mDNS spec says this should never be more than 256 (including trailing '\0').

using namespace ns3;

// A single mDNS Query.
typedef struct Query{
  unsigned int buffer_pointer;            // Position of Answer in packet. (Used for debugging only.)
  char qname_buffer[MAX_MDNS_NAME_LEN];   // Question Name: Contains the object, domain or zone name.
  unsigned int qtype;                     // Question Type: Type of question being asked by client.
  unsigned int qclass;                    // Question Class: Normally the value 1 for Internet (“IN”)
  bool unicast_response;                  //
  bool valid;                             // False if problems were encountered decoding packet.

  void Display() const;                   // Display a summary of this Answer on Serial port.
} Query;

// A single mDNS Answer.
typedef struct Answer{
  unsigned int buffer_pointer;          // Position of Answer in packet. (Used for debugging only.)
  char name_buffer[MAX_MDNS_NAME_LEN];  // object, domain or zone name.
  char rdata_buffer[MAX_MDNS_NAME_LEN]; // The data portion of the resource record.
  unsigned int rrtype;                  // ResourceRecord Type.
  unsigned int rrclass;                 // ResourceRecord Class: Normally the value 1 for Internet (“IN”)
  unsigned long int rrttl;              // ResourceRecord Time To Live: Number of seconds ths should be remembered.
  bool rrset;                           // Flush cache of records matching this name.
  bool valid;                           // False if problems were encountered decoding packet.

  void Display() const ;                // Display a summary of this Answer on Serial port.
} Answer;


class MDns {
 public:

  // Call this regularly to check for an incoming packet.
  //bool Check();

  // Send this MDns packet.
  //void Send() const;

  // Resets everything to reperesent an empty packet.
  // Do this before building a packet for sending.
  void Clear();

  // Add a query to packet prior to sending.
  // May only be done before any Answers have been added.
  void AddQuery(const Query query);

  // Add an answer to packet prior to sending.
  void AddAnswer(const Answer answer);

  // Display a summary of the packet on Serial port.
  void Display() const;

  // Display the raw packet in HEX and ASCII.
  //void DisplayRawPacket() const;

 private:
  struct Query Parse_Query();
  struct Answer Parse_Answer();
  unsigned int PopulateName(const char* name_buffer);
  void PopulateAnswerResult(Answer* answer);

  bool writeToBuffer(const uint8_t value, char* p_name_buffer, int* p_name_buffer_pos, const int name_buffer_len);
  int nameFromDnsPointer(char* p_name_buffer, int name_buffer_pos, const int name_buffer_len,const uint8_t* p_packet_buffer, int packet_buffer_pos, const bool recurse);
  int nameFromDnsPointer(char* p_name_buffer, int name_buffer_pos, const int name_buffer_len,const uint8_t* p_packet_buffer, int packet_buffer_pos);
  int parseText(char* data_buffer, const int data_buffer_len, const int data_len,const uint8_t* p_packet_buffer, int packet_buffer_pos);

  unsigned int data_size;  // Size of mDNS packet.
  unsigned int buffer_pointer;  // Position in data_buffer while processing packet.
  uint8_t data_buffer[MAX_PACKET_SIZE];  // Buffer containing mDNS packet.
  bool type;  // Query or Answer
  bool truncated;  // Whether more follows in another packet.
  unsigned int query_count;  // Number of Qeries in the packet.
  unsigned int answer_count;  // Number of Answers in the packet.
  unsigned int ns_count;
  unsigned int ar_count;
};
#endif  // MDNS_H
