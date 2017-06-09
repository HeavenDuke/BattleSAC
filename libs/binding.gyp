{
    "targets": [
        {
            "target_name": "addon",
            "sources": ["src/hello.cc"],
            "include_dirs": [
            	"<!(node -e \"require('nan')\")"
            ]
        },{
            "target_name": "authentication",
            "sources": ["src/authentication.cc"],
            "libraries": [
            	"<!@(pkg-config --libs libcrypto++)"
            ],
            "include_dirs": [
            	"<!(node -e \"require('nan')\")",
            	"/usr/include/cryptopp"
            ]
        }
    ]
}