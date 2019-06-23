/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Dan Halbert for Adafruit Industries
 * Copyright (c) 2018 Artur Pacholec
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>
#include <stdio.h>

#include "py/objproperty.h"
#include "py/objstr.h"
#include "py/runtime.h"
#include "shared-bindings/bleio/Address.h"
#include "shared-module/bleio/Address.h"

//| .. currentmodule:: bleio
//|
//| :class:`Address` -- BLE address
//| =========================================================
//|
//| Encapsulates the address of a BLE device.
//|

//| .. class:: Address(address, address_type)
//|
//|   Create a new Address object encapsulating the address value.
//|   The value itself can be one of:
//|
//|   :param buf address: The address value to encapsulate. A buffer object (bytearray, bytes) of 6 bytes.
//|   :param int address_type: one of these integers:
//|     - `bleio.Address.PUBLIC` = 0
//|     - `bleio.Address.RANDOM_STATIC` = 1
//|     - `bleio.Address.RANDOM_PRIVATE_RESOLVABLE` = 2
//|     - `bleio.Address.RANDOM_PRIVATE_NON_RESOLVABLE` = 3
//|

STATIC mp_obj_t bleio_address_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_address, ARG_address_type };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_address, MP_ARG_OBJ | MP_ARG_REQUIRED },
        { MP_QSTR_address_type, MP_ARG_INT, {.u_int = BLEIO_ADDRESS_TYPE_PUBLIC } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    bleio_address_obj_t *self = m_new_obj(bleio_address_obj_t);
    self->base.type = &bleio_address_type;

    const mp_obj_t address = args[ARG_address].u_obj;
    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(address, &buf_info, MP_BUFFER_READ);
    if (buf_info.len != NUM_BLEIO_ADDRESS_BYTES) {
        mp_raise_ValueError_varg(translate("Address must be %d bytes long"), NUM_BLEIO_ADDRESS_BYTES);
    }

    const mp_int_t address_type = args[ARG_address_type].u_int;
    if (address_type < BLEIO_ADDRESS_TYPE_MIN || address_type > BLEIO_ADDRESS_TYPE_MAX) {
        mp_raise_ValueError(translate("Address type out of range"));
    }

    common_hal_bleio_address_construct(self, buf_info.buf, address_type);

    return MP_OBJ_FROM_PTR(self);
}

//|   .. attribute:: address_bytes
//|
//|     The bytes that make up the device address (read-only)
//|
//|     - `bleio.Address.PUBLIC`
//|     - `bleio.Address.RANDOM_STATIC`
//|     - `bleio.Address.RANDOM_PRIVATE_RESOLVABLE`
//|     - `bleio.Address.RANDOM_PRIVATE_NON_RESOLVABLE`
//|
STATIC mp_obj_t bleio_address_get_address_bytes(mp_obj_t self_in) {
    bleio_address_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return common_hal_bleio_address_get_address_bytes(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(bleio_address_get_address_bytes_obj, bleio_address_get_address_bytes);

const mp_obj_property_t bleio_address_address_bytes_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&bleio_address_get_address_bytes_obj,
              (mp_obj_t)&mp_const_none_obj,
              (mp_obj_t)&mp_const_none_obj},
};

//|   .. attribute:: type
//|
//|     The address type (read-only). One of these integers:
//|
//|     - `bleio.Address.PUBLIC`
//|     - `bleio.Address.RANDOM_STATIC`
//|     - `bleio.Address.RANDOM_PRIVATE_RESOLVABLE`
//|     - `bleio.Address.RANDOM_PRIVATE_NON_RESOLVABLE`
//|
STATIC mp_obj_t bleio_address_get_type(mp_obj_t self_in) {
    bleio_address_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return MP_OBJ_NEW_SMALL_INT(common_hal_bleio_address_get_type(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(bleio_address_get_type_obj, bleio_address_get_type);

const mp_obj_property_t bleio_address_type_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&bleio_address_get_type_obj,
              (mp_obj_t)&mp_const_none_obj,
              (mp_obj_t)&mp_const_none_obj},
};

//|   .. method:: __eq__(other)
//|
//|     Two Address objects are equal if their addresses and address types are equal.
//|
STATIC mp_obj_t bleio_address_binary_op(mp_binary_op_t op, mp_obj_t lhs_in, mp_obj_t rhs_in) {
    switch (op) {
        // Two Addresses are equal if their address bytes and address_type are equal
        case MP_BINARY_OP_EQUAL:
            if (MP_OBJ_IS_TYPE(rhs_in, &bleio_address_type)) {
                bleio_address_obj_t *lhs = MP_OBJ_TO_PTR(lhs_in);
                bleio_address_obj_t *rhs = MP_OBJ_TO_PTR(rhs_in);
                return mp_obj_new_bool(
                    mp_obj_equal(common_hal_bleio_address_get_address_bytes(lhs),
                                 common_hal_bleio_address_get_address_bytes(rhs)) &&
                    common_hal_bleio_address_get_type(lhs) ==
                    common_hal_bleio_address_get_type(rhs));

            } else {
                return mp_const_false;
            }

        default:
            return MP_OBJ_NULL; // op not supported
    }
}

STATIC void bleio_address_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    bleio_address_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_obj_t address_bytes = common_hal_bleio_address_get_address_bytes(self);

    mp_buffer_info_t buf_info;
    mp_get_buffer_raise(address_bytes, &buf_info, MP_BUFFER_READ);
    const uint8_t *buf = (uint8_t *) buf_info.buf;
    mp_printf(print,
              "%02x:%02x:%02x:%02x:%02x:%02x",
              buf[5], buf[4], buf[3], buf[2], buf[1], buf[0]);
}

STATIC const mp_rom_map_elem_t bleio_address_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_address_bytes), MP_ROM_PTR(&bleio_address_address_bytes_obj) },
    { MP_ROM_QSTR(MP_QSTR_type), MP_ROM_PTR(&bleio_address_type_obj) },
    // These match the BLE_GAP_ADDR_TYPES values used by the nRF library.
    { MP_ROM_QSTR(MP_QSTR_PUBLIC), MP_OBJ_NEW_SMALL_INT(0) },
    { MP_ROM_QSTR(MP_QSTR_RANDOM_STATIC), MP_OBJ_NEW_SMALL_INT(1) },
    { MP_ROM_QSTR(MP_QSTR_RANDOM_PRIVATE_RESOLVABLE), MP_OBJ_NEW_SMALL_INT(2) },
    { MP_ROM_QSTR(MP_QSTR_RANDOM_PRIVATE_NON_RESOLVABLE), MP_OBJ_NEW_SMALL_INT(3) },

};

STATIC MP_DEFINE_CONST_DICT(bleio_address_locals_dict, bleio_address_locals_dict_table);

const mp_obj_type_t bleio_address_type = {
    { &mp_type_type },
    .name = MP_QSTR_Address,
    .make_new = bleio_address_make_new,
    .print = bleio_address_print,
    .binary_op = bleio_address_binary_op,
    .locals_dict = (mp_obj_dict_t*)&bleio_address_locals_dict
};
