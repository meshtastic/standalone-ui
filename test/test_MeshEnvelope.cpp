#include "comms/MeshEnvelope.h"
#include "meshtastic/config.pb.h"
#include "meshtastic/mesh.pb.h"
#include <unity.h>

uint8_t buffer[512];
char* buf = (char*)buffer;
size_t size;
size_t payload;


void tearDown(void) {
    // clean stuff up here
}

void test_MeshEnvelope_ctor1(void) {
    MeshEnvelope envelope;
}

void test_MeshEnvelope_ctor2(void) {
    strcpy(buf, "\x01");
    TEST_ASSERT_EQUAL(42, 42);
}

void test_MeshEnvelope_validate_size(void) {
    strcpy(buf, "\x01\x00");
    size = 1;
    TEST_ASSERT_FALSE(MeshEnvelope::validate(buffer, size, payload));
}

void test_MeshEnvelope_validate_wrong_magic1(void) {
    strcpy(buf, "\x01\x00");
    size = 5;
    TEST_ASSERT_FALSE(MeshEnvelope::validate(buffer, size, payload));
}

void test_MeshEnvelope_validate_wrong_magic2(void) {
    strcpy(buf, "\x94\x94\x00");
    size = 5;
    TEST_ASSERT_FALSE(MeshEnvelope::validate(buffer, size, payload));
}

void test_MeshEnvelope_validate_wrong_size(void) {
    memcpy(buf, "\x94\xc3\x00\x02", 4);
    size = 5;
    TEST_ASSERT_FALSE(MeshEnvelope::validate(buffer, size, payload));
    TEST_ASSERT_EQUAL(2, payload);
}

void test_MeshEnvelope_validate_right_size(void) {
    memcpy(buf, "\x94\xc3\x00\x01", 4);
    size = 5;
    TEST_ASSERT_TRUE(MeshEnvelope::validate(buffer, size, payload));
    TEST_ASSERT_EQUAL(payload, 1);
}

void test_MeshEnvelope_validate_partial_packet(void) {
    memcpy(buf, "\x94\xc3\x00\x03", 4);
    size = 5;
    TEST_ASSERT_FALSE(MeshEnvelope::validate(buffer, size, payload));
    TEST_ASSERT_EQUAL(3, payload);
}

void test_MeshEnvelope_validate_skip_bytes(void) {
    memcpy(buf, "\x22\x23\x24\x94\xc3\x00\x04", 7);
    size = 9;
    TEST_ASSERT_FALSE(MeshEnvelope::validate(buffer, size, payload));
    TEST_ASSERT_EQUAL(4, payload);
    TEST_ASSERT_EQUAL(0x94, buffer[0]);
    TEST_ASSERT_EQUAL(0xc3, buffer[1]);
    TEST_ASSERT_EQUAL(0x00, buffer[2]);
    TEST_ASSERT_EQUAL(0x04, buffer[3]);
}

void test_MeshEnvelope_validate_packet_with_skip_bytes(void) {
    memcpy(buf, "\x22\x23\x24\x94\xc3\x00\x05", 7);
    size = 15;
    TEST_ASSERT_TRUE(MeshEnvelope::validate(buffer, size, payload));
    TEST_ASSERT_EQUAL(5, payload);
    TEST_ASSERT_EQUAL(0x94, buffer[0]);
    TEST_ASSERT_EQUAL(0xc3, buffer[1]);
    TEST_ASSERT_EQUAL(0x00, buffer[2]);
    TEST_ASSERT_EQUAL(0x05, buffer[3]);
}

void test_packet_authenticity_schema_contract(void) {
    meshtastic_Config_SecurityConfig security = meshtastic_Config_SecurityConfig_init_default;
    meshtastic_DeviceMetadata metadata = meshtastic_DeviceMetadata_init_zero;

    TEST_ASSERT_EQUAL(9, meshtastic_Config_SecurityConfig_packet_signature_policy_tag);
    TEST_ASSERT_EQUAL(14, meshtastic_DeviceMetadata_has_xeddsa_tag);
    TEST_ASSERT_EQUAL(meshtastic_Config_SecurityConfig_PacketSignaturePolicy_PACKET_SIGNATURE_POLICY_BALANCED,
                      security.packet_signature_policy);
    TEST_ASSERT_FALSE(metadata.has_xeddsa);
    TEST_ASSERT_EQUAL(1, meshtastic_Config_SecurityConfig_PacketSignaturePolicy_PACKET_SIGNATURE_POLICY_COMPATIBLE);
    TEST_ASSERT_EQUAL(2, meshtastic_Config_SecurityConfig_PacketSignaturePolicy_PACKET_SIGNATURE_POLICY_STRICT);
}


void RUN_UNITY_TESTS() {
    UNITY_BEGIN();
    RUN_TEST(test_MeshEnvelope_ctor1);
    RUN_TEST(test_MeshEnvelope_ctor2);
    RUN_TEST(test_MeshEnvelope_validate_size);
    RUN_TEST(test_MeshEnvelope_validate_wrong_magic1);
    RUN_TEST(test_MeshEnvelope_validate_wrong_magic2);
    RUN_TEST(test_MeshEnvelope_validate_wrong_size);
    RUN_TEST(test_MeshEnvelope_validate_right_size);
    RUN_TEST(test_MeshEnvelope_validate_partial_packet);
    RUN_TEST(test_MeshEnvelope_validate_skip_bytes);
    RUN_TEST(test_MeshEnvelope_validate_packet_with_skip_bytes);
    RUN_TEST(test_packet_authenticity_schema_contract);
    UNITY_END();
}

#ifdef ARDUINO

#include <Arduino.h>
void setup() {
    size = 0;
    payload = 0;
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    //delay(2000);

    RUN_UNITY_TESTS();
}

void loop() {
//    digitalWrite(13, HIGH);
//    delay(100);
//    digitalWrite(13, LOW);
//    delay(500);
}

#else

int main(int argc, char **argv) {
    RUN_UNITY_TESTS();
    return 0;
}

#endif
