# Copyright (c) 2015-2018 Rafael Leira
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Directories
set(certs_dir "${CMAKE_BINARY_DIR}/certs")
set(certs_prv_dir "${CMAKE_BINARY_DIR}/certs/prv")

# Cert info
set(certs_info_ca -subj "/C=ES/ST=Madrid/L=Madrid/O=WormHole.ca/CN=www.wormhole.org")
set(certs_info_ei -subj "/C=ES/ST=Madrid/L=Madrid/O=WormHole.zeus/CN=www.wormhole.org")
set(certs_info_wh -subj "/C=ES/ST=Madrid/L=Madrid/O=WormHole.worm/CN=www.wormhole.org")
set(certs_curve   "brainpoolP512r1") # other recomended if the first fails: secp521r1
set(certs_hash    "sha512")
set(certs_days2die   "512")

# Openssl command
set(certs_openssl "openssl")

# Openssl DIR

if (WH_SSLCERTS)

    add_custom_target("certs-ca"            DEPENDS "${certs_dir}/ca.pem")
    add_custom_target("certs-zeus"      DEPENDS "${certs_dir}/zeus.pem" "certs-ca")
    add_custom_target("certs-worm"          DEPENDS "${certs_dir}/worm.pem"     "certs-ca")
    add_custom_target("certificates"    ALL DEPENDS                             "certs-ca" "certs-worm" "certs-zeus")

    # create the CA
    add_custom_command(
        OUTPUT "${certs_dir}/ca.pem" "${certs_prv_dir}/ca.key.pem" # "${certs_prv_dir}/ca.csr" 

        COMMAND mkdir -p ${certs_dir}
        COMMAND mkdir -p ${certs_prv_dir}

        COMMAND ${certs_openssl} ecparam -name ${certs_curve} -genkey -noout -out "${certs_prv_dir}/ca.key.pem"
        COMMAND ${certs_openssl} req ${certs_info_ca} -x509 -new -nodes -key "${certs_prv_dir}/ca.key.pem" -${certs_hash} -days ${certs_days2die}  -extensions v3_ca -out "${certs_dir}/ca.pem"

        VERBATIM
    )

    # create the zeus
    add_custom_command(
        OUTPUT "${certs_dir}/zeus.pem" "${certs_prv_dir}/zeus.key.pem" "${certs_prv_dir}/zeus.csr" 

        COMMAND ${certs_openssl} ecparam -name ${certs_curve} -genkey -noout -out "${certs_prv_dir}/zeus.key.pem"
        COMMAND ${certs_openssl} req ${certs_info_ei} -new -key "${certs_prv_dir}/zeus.key.pem" -out "${certs_prv_dir}/zeus.csr"
        COMMAND ${certs_openssl} x509 -req -in "${certs_prv_dir}/zeus.csr" -CA "${certs_dir}/ca.pem" -CAkey "${certs_prv_dir}/ca.key.pem" -CAcreateserial -out "${certs_dir}/zeus.pem" -days ${certs_days2die} -${certs_hash}

        DEPENDS "${certs_dir}/ca.pem" "${certs_prv_dir}/ca.key.pem"
        VERBATIM
    )

    # create the WORM
    add_custom_command(
        OUTPUT "${certs_dir}/worm.pem" "${certs_prv_dir}/worm.key.pem" "${certs_prv_dir}/worm.csr" 

        COMMAND ${certs_openssl} ecparam -name ${certs_curve} -genkey -noout -out "${certs_prv_dir}/worm.key.pem"
        COMMAND ${certs_openssl} req ${certs_info_ei} -new -key "${certs_prv_dir}/worm.key.pem" -out "${certs_prv_dir}/worm.csr"
        COMMAND ${certs_openssl} x509 -req -in "${certs_prv_dir}/worm.csr" -CA "${certs_dir}/ca.pem" -CAkey "${certs_prv_dir}/ca.key.pem" -CAcreateserial -out "${certs_dir}/worm.pem" -days ${certs_days2die} -${certs_hash}

        DEPENDS "${certs_dir}/ca.pem" "${certs_prv_dir}/ca.key.pem"
        VERBATIM
    )
endif()