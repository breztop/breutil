function(link_libpqxx target)
    find_package(PostgreSQL REQUIRED)

    find_package(libpqxx CONFIG QUIET)
    if(TARGET libpqxx::pqxx)
        target_link_libraries(${target} PRIVATE libpqxx::pqxx PostgreSQL::PostgreSQL)
        return()
    endif()

    if(TARGET pqxx::pqxx)
        target_link_libraries(${target} PRIVATE pqxx::pqxx PostgreSQL::PostgreSQL)
        return()
    endif()

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBPQXX REQUIRED IMPORTED_TARGET libpqxx)
    target_link_libraries(${target} PRIVATE PkgConfig::LIBPQXX PostgreSQL::PostgreSQL)
endfunction()
