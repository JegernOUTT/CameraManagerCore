  find_library(ONVIFWRAPPER_STATIC_LIBRARY
    NAMES
      onvif_wrapper_static
    PATHS
      /usr/lib
      /usr/local/lib
  )

  find_library(ONVIFGSOAP_STATIC_LIBRARY
    NAMES
      onvif_gsoap_static
    PATHS
      /usr/lib
      /usr/local/lib
  )

  find_library(ONVIFWRAPPER_SHARED_LIBRARY
    NAMES
      onvif_wrapper
    PATHS
      /usr/lib
      /usr/local/lib
  )

  find_library(ONVIFGSOAP_SHARED_LIBRARY
    NAMES
      onvif_gsoap
    PATHS
      /usr/lib
      /usr/local/lib
  )
