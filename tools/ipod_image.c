#include "imagetool.h"
#include  "mkimage.h"
#include <image.h>

struct image1_header {
    char magic[4]; // 0x00
    char version[3]; // 0x04
    uint8_t format; // 0x07
    uint32_t entrypoint; // 0x08
    uint32_t body_len; // 0x0c 
    uint32_t data_len; // 0x10
    uint32_t footer_cert_offset; // 0x14
    uint32_t footer_cert_len; // 0x18
    uint8_t salt[32]; // 0x1c
    uint16_t unk1; // 0x3c
    uint16_t unk2; // 0x3e
    uint8_t header_signature[16]; // 0x40
    uint8_t padding[0x600 - 0x50]; // 0x50
};

static struct image1_header ipodimage_header;

static int ipodimage_check_params(struct image_tool_params *params)
{
    if (!params)
        return 0;

    return 0;
}

static int ipodimage_verify_header(unsigned char *ptr, int image_size,
        struct image_tool_params *params)
{
    struct image1_header *hdr = (struct image1_header *)ptr;

    if (image_size < sizeof(struct image1_header))
        return -1;

    if (memcmp(hdr->magic, "8730", 4) != 0)
        return -1;

    if (memcmp(hdr->version, "2.0", 3) != 0)
        return -1;

    switch (hdr->format) {
    case 2: // SIGNED
    case 4: // X509_SIGNED
        break;
    default:
        return -1;
    }
    
    return 0;
}

static void ipodimage_print_header(const void *ptr)
{
    struct image1_header *hdr = (struct image1_header *)ptr;
    printf("Image Type    : Apple/Samsung S5L IMG1\n");
}

static void ipodimage_set_header(void *ptr, struct stat *sbuf, int ifd,
        struct image_tool_params *params)
{
    struct image1_header *hdr = (struct image1_header *)ptr;
    memcpy(hdr->magic, "8730", 4);
    memcpy(hdr->version, "2.0", 3);
    hdr->format = 4;
    hdr->entrypoint = 0;
    hdr->body_len = cpu_to_le32((uint32_t)sbuf->st_size);
    hdr->data_len = cpu_to_le32((uint32_t)sbuf->st_size + 0x80 + 0x300);
    hdr->footer_cert_offset = cpu_to_le32((uint32_t)sbuf->st_size + 0x80);
    hdr->footer_cert_len = cpu_to_le32(0x300);
}

static int ipodimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_IPODIMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

U_BOOT_IMAGE_TYPE(
    ipodimage,
    "Apple iPod Image1 support",
    sizeof(struct image1_header),
    (void *)&ipodimage_header,
    ipodimage_check_params,
    ipodimage_verify_header,
    ipodimage_print_header,
    ipodimage_set_header,
    NULL,
    ipodimage_check_image_types,
    NULL,
    NULL
);
