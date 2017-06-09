{
    "targets": [
    	{
            "target_name": "authentication",
            "sources": ["src/authentication.cc"],
            "libraries": [
            	"<!@(pkg-config --libs libcrypto++)"
            ],
            "include_dirs": [
            	"<!(node -e \"require('nan')\")",
            	"/usr/include/cryptopp"
            ]
        },{
            "target_name": "supplement",
            "sources": ["src/supplement.cc"],
            "libraries": [
            	"<!@(pkg-config --libs libcrypto++)"
            ],
            "include_dirs": [
            	"<!(node -e \"require('nan')\")",
            	"/usr/include/cryptopp"
            ]
        },{
            "target_name": "voting",
            "sources": ["src/voting.cc"],
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