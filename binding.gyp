{
  "targets": [
    {
      "target_name": "addon",
      "sources": [
        "addon.cc",
        "sdbcl.cc",
        "v8qpack.cc"
      ],
      "link_settings": {
        "libraries": [
          "-lsiridb",
          "-lqpack",
          "-lsuv",
          "-luv"
        ]
      },
      "cflags_cc": [
        "-fexceptions"
      ]
    }
  ]
  
}