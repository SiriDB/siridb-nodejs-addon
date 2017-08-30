{
  "targets": [
    {
      "target_name": "addon",
      "sources": [
        "addon.cc",
        "sdbcl.cc",
        "suv.c"
      ],
      "link_settings": {
        "libraries": [
          "-lsiridb"
        ]
      },
      "cflags_cc": [
        "-fexceptions"
      ]      
    }
  ]
  
}