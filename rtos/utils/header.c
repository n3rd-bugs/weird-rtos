/*
 * header.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <os.h>
#include <header.h>
#include <string.h>

/*
 * header_gen_machine_init
 * @machine: Header machine needed to be initialized.
 * @push: Function that will be called to push data on the buffer.
 * This function will initialize a header generator machine for further
 * processing.
 */
void header_gen_machine_init(HDR_GEN_MACHINE *machine, HRD_PUSH *push)
{
    /* Clear the machine data. */
    memset(machine, 0, sizeof(HDR_GEN_MACHINE));
    machine->push = push;

} /* header_gen_machine_init */

/*
 * header_generate
 * @machine: Header generator machine instance.
 * @headers: Header data that will be populated in the given buffer.
 * @num_headers: Number of headers needed to be generated.
 * @buffer: Buffer in which header will be generated.
 * @return: A success status will be returned if the header was successfully
 *  generated.
 * This function will generate a header according to the provided header
 * definition.
 */
int32_t header_generate(HDR_GEN_MACHINE *machine, HEADER *header, uint32_t num_headers, void *buffer)
{
    int32_t status = SUCCESS;

    /* Pick the last header. */
    header += (num_headers - 1);

    /* While we have a header to process and we have a success status. */
    while ((status == SUCCESS) && (num_headers > 0))
    {
        /* Push this header on the buffer. */
        status = machine->push(buffer, header->value, header->size, header->flags);

        /* Pick the next header. */
        header--;
        num_headers--;
    }

    /* Return status to the caller. */
    return (status);

} /* header_generate */

/*
 * header_parse_machine_init
 * @machine: Header machine needed to be initialized.
 * @pull: Function that will be called to pull data from the buffer.
 * This function will initialize a header parser machine for further processing.
 */
void header_parse_machine_init(HDR_PARSE_MACHINE *machine, HRD_PULL *pull)
{
    /* Clear the machine data. */
    memset(machine, 0, sizeof(HDR_PARSE_MACHINE));
    machine->pull = pull;

} /* header_parse_machine_init */

/*
 * header_parse
 * @machine: Header parser machine instance.
 * @headers: Header data that will be updated as the header is parsed.
 * @num_headers: Number of headers needed to be parsed.
 * @buffer: Buffer from which given header will be parsed.
 * @return: A success status will be returned if the header was successfully
 *  generated.
 * This function will parse a header according to the provided header
 * definition.
 */
int32_t header_parse(HDR_PARSE_MACHINE *machine, HEADER *header, uint32_t num_headers, void *buffer)
{
    int32_t status = SUCCESS;

    /* While we have a header to process and we have a success status. */
    while ((status == SUCCESS) && (num_headers > 0))
    {
        /* Pull this header from the buffer. */
        status = machine->pull(buffer, header->value, header->size, header->flags);

        /* Pick the next header. */
        header++;
        num_headers--;
    }

    /* Return status to the caller. */
    return (status);

} /* header_parse */

/*
 * header_process_machine_init
 * @machine: Header machine needed to be initialized.
 * @pull: Function that will be called to pull data from the buffer.
 * This function will initialize a header process machine.
 */
void header_process_machine_init(HDR_PROCESS_MACHINE *machine, HRD_PULL *pull)
{
    /* Clear the machine data. */
    memset(machine, 0, sizeof(HDR_PROCESS_MACHINE));
    machine->pull = pull;

} /* header_process_machine_init */

/*
 * header_process_machine_run
 * @machine: Header machine instance.
 * @data: Data needed to be forwarded to the process function.
 * @header: Header definition being processed.
 * @buffer: Buffer from which header is needed to be processed.
 * @proc_buf: Process buffer that will be used to pass data to the process
 *  function.
 * This function will run the header machine that will parse and process the
 * given header.
 */
int32_t header_process_machine_run(HDR_PROCESS_MACHINE *machine, void *data, const HEADER *header, void *buffer, uint8_t *proc_buf)
{
    int32_t status = SUCCESS;
    const HEADER *header_ptr = &header[machine->header];
    uint32_t bit_size, byte_size, old_num_bits, i;

    /* While we have a header to process and we have a success status. */
    while ((status == SUCCESS) && ((header_ptr->flags & HEADER_END) == 0))
    {
        /* Pick the size. */
        bit_size = header_ptr->size;

        /* Check if this was a byte header. */
        if ((header_ptr->flags & HEADER_BIT) == 0)
        {
            /* Convert the size to number of bits we need to pick. */
            bit_size *= 8;
        }

        /* Calculate number of bytes we need to pull. */
        byte_size = (ALLIGN_CEIL_N((bit_size - machine->num_bits), 8) / 8);

        /* If we have some data left from previous pull. */
        if (machine->num_bits)
        {
            /* Push the last byte of the previous pull. */
            *proc_buf = machine->byte;
        }

        /* Pull the required number of bytes. */
        status = machine->pull(buffer, proc_buf + (machine->num_bits != 0), byte_size, 0);

        if (status == SUCCESS)
        {
            /* If we have unaligned data. */
            if ((bit_size % 8) || (machine->num_bits != 0))
            {
                /* If we did pull some data. */
                if (byte_size)
                {
                    /* Save the last byte of this pull. */
                    machine->byte = proc_buf[byte_size - (machine->num_bits == 0)];
                }

                /* If there are some valid bits left and this value is not in
                 * the form of a packet structure (i.e. align data to the left). */
                if (machine->num_bits)
                {
                    /* Align the data in the process buffer to the left. */
                    for (i = 0; i < byte_size; i++)
                    {
                        /* Just shift left the whole buffer here by the number
                         * of bit we don't need on the buffer i.e.
                         * (8 - machine->num_bits). */
                        proc_buf[i] = (uint8_t)((proc_buf[i] << ((8 - machine->num_bits) % 8)) | (proc_buf[i + 1] >> machine->num_bits));
                    }

                    /* Align data in last byte. */
                    proc_buf[byte_size] = (uint8_t)(proc_buf[byte_size] << ((8 - machine->num_bits) % 8));
                }

                /* Save the number of bits still valid. */
                old_num_bits = machine->num_bits;
                machine->num_bits = (uint8_t)(((uint8_t)(8 + machine->num_bits) - bit_size) % 8);

                /* If there are some valid bits left on the last byte that are
                 * needed to be processed in next read. */
                if (machine->num_bits)
                {
                    /* Remove the valid bits for the next read. */
                    proc_buf[(ALLIGN_FLOOR_N(bit_size, 8) / 8)] &= (uint8_t)~((1 << ((machine->num_bits + ((8 - old_num_bits) % 8)) % 8)) - 1);

                    /* Check if we need to clear next byte too. */
                    if ((machine->num_bits + ((8 - old_num_bits) % 8)) >= 8)
                    {
                        /* Clear the next byte too. */
                        proc_buf[(ALLIGN_FLOOR_N(bit_size, 8) / 8) + 1] = 0;
                    }
                }
            }

            /* If we need to process the header value. */
            if (header_ptr->flags & HEADER_PROCESS)
            {
                /* If this is not a bit field. */
                if ((header_ptr->flags & HEADER_BIT) == 0)
                {
                    /* Convert size to number of bytes. */
                    bit_size /= 8;
                }

                /* Process this header value. */
                ((HRD_PROCESS *)header_ptr->value)(data, proc_buf, bit_size);
            }

            /* Process next header. */
            header_ptr++;
        }
    }

    /* Return status to the caller. */
    return (status);

} /* header_process_machine_run */
