# Directories
set(certs_dir "${CMAKE_BINARY_DIR}/certs")
set(certs_prv_dir "${CMAKE_BINARY_DIR}/certs/prv")

# Cert info
set(certs_info_ca -subj "/C=ES/ST=Madrid/L=Madrid/O=WormHole.ca/CN=www.wormhole.org")
set(certs_info_ei -subj "/C=ES/ST=Madrid/L=Madrid/O=WormHole.einstein/CN=www.wormhole.org")
set(certs_info_wh -subj "/C=ES/ST=Madrid/L=Madrid/O=WormHole.worm/CN=www.wormhole.org")
set(certs_curve   "brainpoolP512r1") # other recomended if the first fails: secp521r1
set(certs_hash    "sha512")
set(certs_days2die   "512")

# Openssl command
set(certs_openssl "openssl")

# Openssl DIR

if (WH_SSLCERTS)

    add_custom_target("certs-ca"            DEPENDS "${certs_dir}/ca.pem")
    add_custom_target("certs-einstein"      DEPENDS "${certs_dir}/einstein.pem" "certs-ca")
    add_custom_target("certs-worm"          DEPENDS "${certs_dir}/worm.pem"     "certs-ca")
    add_custom_target("certificates"    ALL DEPENDS                             "certs-ca" "certs-worm" "certs-einstein")

    # create the CA
    add_custom_command(
        OUTPUT "${certs_dir}/ca.pem" "${certs_prv_dir}/ca.key.pem" # "${certs_prv_dir}/ca.csr" 

        COMMAND mkdir -p ${certs_dir}
        COMMAND mkdir -p ${certs_prv_dir}

        COMMAND ${certs_openssl} ecparam -name ${certs_curve} -genkey -noout -out "${certs_prv_dir}/ca.key.pem"
        COMMAND ${certs_openssl} req ${certs_info_ca} -x509 -new -nodes -key "${certs_prv_dir}/ca.key.pem" -${certs_hash} -days ${certs_days2die}  -extensions v3_ca -out "${certs_dir}/ca.pem"

        VERBATIM
    )

    # create the EINSTEIN
    add_custom_command(
        OUTPUT "${certs_dir}/einstein.pem" "${certs_prv_dir}/einstein.key.pem" "${certs_prv_dir}/einstein.csr" 

        COMMAND ${certs_openssl} ecparam -name ${certs_curve} -genkey -noout -out "${certs_prv_dir}/einstein.key.pem"
        COMMAND ${certs_openssl} req ${certs_info_ei} -new -key "${certs_prv_dir}/einstein.key.pem" -out "${certs_prv_dir}/einstein.csr"
        COMMAND ${certs_openssl} x509 -req -in "${certs_prv_dir}/einstein.csr" -CA "${certs_dir}/ca.pem" -CAkey "${certs_prv_dir}/ca.key.pem" -CAcreateserial -out "${certs_dir}/einstein.pem" -days ${certs_days2die} -${certs_hash}

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