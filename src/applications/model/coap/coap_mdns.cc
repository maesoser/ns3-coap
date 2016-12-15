#include "coap_node.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Coap_mDNS");


void MDns::Clear() {
  data_buffer[0] = 0;     // Query ID field which is unused in mDNS.
  data_buffer[1] = 0;     // Query ID field which is unused in mDNS.
  data_buffer[2] = 0;     // 0b00000000 for Query, 0b10000000 for Answer.
  data_buffer[3] = 0;     // DNS flags which are mostly unused in mDNS.
  data_buffer[4] = 0;     // Number of queries.
  data_buffer[5] = 0;     // Number of queries.
  data_buffer[6] = 0;     // Number of answers.
  data_buffer[7] = 0;     // Number of answers.
  data_buffer[8] = 0;     // Number of Server esource records.
  data_buffer[9] = 0;     // Number of Server esource records.
  data_buffer[10] = 0;     // Number of Additional resource records.
  data_buffer[11] = 0;     // Number of Additional resource records.

  data_size = 12;
  buffer_pointer = 12;  // First byte of first Query/Record.
  type = 0;
  query_count = 0;
  answer_count = 0;
  ns_count = 0;
  ar_count = 0;
}

unsigned int MDns::PopulateName(const char* name_buffer) {
  // TODO: This section does not match the full mDNS spec
  // as it does not re-use strings from previous queries.

  unsigned int buffer_pointer_start = buffer_pointer;
  int word_start = 0, word_end = 0;
  do {
    if (name_buffer[word_end] == '.' or name_buffer[word_end] == '\0') {
      const int word_length = word_end - word_start;
      data_buffer[buffer_pointer++] = (unsigned uint8_t)word_length;
      for (int i = word_start; i < word_end; ++i) {
        data_buffer[buffer_pointer++] = name_buffer[i];
      }
      word_end++;  // Skip the '.' character.
      word_start = word_end;
    }
    word_end++;
  } while (name_buffer[word_start] != '\0');
  data_buffer[buffer_pointer++] = '\0';  // End of qname.

  return buffer_pointer - buffer_pointer_start;
}

void MDns::AddQuery(const Query query) {
  if (answer_count || ns_count || ar_count) {
    NS_LOG_INFO(" ERROR. Resource records inclued before Queries.");
    return;
  }
  data_buffer[2] = 0;     // 0b00000000 for Query, 0b10000000 for Answer.
  type = 1;
  ++query_count;
  data_buffer[4] = (query_count & 0xFF00) >> 8;
  data_buffer[5] = query_count & 0xFF;

  // Create DNS name buffer from qname.
  PopulateName(query.qname_buffer);

  // The rest of the flags.
  data_buffer[buffer_pointer++] = (query.qtype & 0xFF00) >> 8;
  data_buffer[buffer_pointer++] = query.qtype & 0xFF;
  unsigned int qclass = 0;
  if (query.unicast_response) {
    qclass = 0b1000000000000000;
  }
  qclass += query.qclass;
  data_buffer[buffer_pointer++] = (qclass & 0xFF00) >> 8;
  data_buffer[buffer_pointer++] = qclass & 0xFF;
  data_size = buffer_pointer;
}

void MDns::AddAnswer(const Answer answer) {
  if (ns_count || ar_count) {
    NS_LOG_INFO(" ERROR. NS or AR records added before Answer records");
    return;
  }
  answer_count++;
  data_buffer[6] = (answer_count & 0xFF00) >> 8;
  data_buffer[7] = answer_count & 0xFF;

  // Create DNS name buffer from name.
  PopulateName(answer.name_buffer);

  data_buffer[buffer_pointer++] = (answer.rrtype & 0xFF00) >> 8;
  data_buffer[buffer_pointer++] = answer.rrtype & 0xFF;

  unsigned int rrclass = 0;
  if (answer.rrset) {
    rrclass = 0b1000000000000000;
  }
  rrclass += answer.rrclass;
  data_buffer[buffer_pointer++] = (rrclass & 0xFF00) >> 8;
  data_buffer[buffer_pointer++] = rrclass & 0xFF;

  data_buffer[buffer_pointer++] = (answer.rrttl & 0xFF000000) >> 24;
  data_buffer[buffer_pointer++] = (answer.rrttl & 0xFF0000) >> 16;
  data_buffer[buffer_pointer++] = (answer.rrttl & 0xFF00) >> 8;
  data_buffer[buffer_pointer++] = (answer.rrttl & 0xFF);

  const unsigned int rdata_len_p0 = buffer_pointer++;
  const unsigned int rdata_len_p1 = buffer_pointer++;
  unsigned int rdata_len;

  switch (answer.rrtype) {
    case MDNS_TYPE_A:  // Returns a 32-bit IPv4 address
      rdata_len = 4;
      data_buffer[buffer_pointer++] = answer.rdata_buffer[0];
      data_buffer[buffer_pointer++] = answer.rdata_buffer[1];
      data_buffer[buffer_pointer++] = answer.rdata_buffer[2];
      data_buffer[buffer_pointer++] = answer.rdata_buffer[3];
      break;
    case MDNS_TYPE_PTR:  // Pointer to a canonical name.
      rdata_len = PopulateName(answer.rdata_buffer);
      break;
    default:
      // TODO: Other record types.
      NS_LOG_INFO(" **ERROR** Sending this record type not implemented yet.");
  }

  data_buffer[rdata_len_p0] = (rdata_len & 0xFF00) >> 8;
  data_buffer[rdata_len_p1] = rdata_len & 0xFF;

  data_size = buffer_pointer;
}

void MDns::Display() const {
  NS_LOG_INFO("Packet size: ");
  NS_LOG_INFO(data_size);
  NS_LOG_INFO(" TYPE: ");
  NS_LOG_INFO(type);
  NS_LOG_INFO("      QUERY_COUNT: ");
  NS_LOG_INFO(query_count);
  NS_LOG_INFO("      ANSWER_COUNT: ");
  NS_LOG_INFO(answer_count);
  NS_LOG_INFO("      NS_COUNT: ");
  NS_LOG_INFO(ns_count);
  NS_LOG_INFO("      AR_COUNT: ");
  NS_LOG_INFO(ar_count);
}
