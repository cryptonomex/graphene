Intro for new developers
------------------------

This is a quick introduction to get new developers up to speed on Graphene.

Starting Graphene
-----------------

For Ubuntu 14.04 LTS users, see this link first:
    https://github.com/cryptonomex/graphene/wiki/build-ubuntu

and then proceed with:

    sudo apt-get update
    sudo apt-get install autoconf autotools-dev build-essential cmake g++ git libboost-all-dev libboost-dev libbz2-dev libdb++-dev libdb-dev libicu-dev libreadline-dev libssl-dev libtool openssl python-dev uuid-dev
    git clone https://github.com/cryptonomex/secp256k1-zkp
    git clone https://github.com/cryptonomex/graphene.git
    cd graphene
    git submodule update --init --recursive
    cmake -DCMAKE_BUILD_TYPE=Debug .
    make
    ./programs/witness_node/witness_node

This will launch the witness node. If you would like to launch the command-line wallet, you must first specify a port
for communication with the witness node. To do this, add text to `witness_node_data_dir/config.ini` as follows, then
restart the node:

    rpc-endpoint = 127.0.0.1:8090

Then, in a separate terminal window, start the command-line wallet `cli_wallet`:

    ./programs/cli_wallet/cli_wallet

To set your iniital password to 'password' use:

    >>> set_password password
    >>> unlock password

To import your initial balance:

    >>> import_balance nathan [5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3] true

If you send private keys over this connection, `rpc-endpoint` should be bound to localhost for security.

A list of CLI wallet commands is available
[here](https://github.com/cryptonomex/graphene/blob/master/libraries/wallet/include/graphene/wallet/wallet.hpp).

Code coverage testing
---------------------

Check how much code is covered by unit tests, using gcov/lcov (see http://ltp.sourceforge.net/coverage/lcov.php ).

    cmake -D ENABLE_COVERAGE_TESTING=true -D CMAKE_BUILD_TYPE=Debug .
    make
    lcov --capture --initial --directory . --output-file base.info --no-external
    libraries/fc/bloom_test
    libraries/fc/task_cancel_test
    libraries/fc/api
    libraries/fc/blind
    libraries/fc/ecc_test test
    libraries/fc/real128_test
    libraries/fc/lzma_test README.md
    libraries/fc/ntp_test
    tests/intense_test
    tests/app_test
    tests/chain_bench
    tests/chain_test
    tests/performance_test
    lcov --capture --directory . --output-file test.info --no-external
    lcov --add-tracefile base.info --add-tracefile test.info --output-file total.info
    lcov -o interesting.info -r total.info libraries/fc/vendor/\* libraries/fc/tests/\* tests/\*
    mkdir -p lcov
    genhtml interesting.info --output-directory lcov --prefix `pwd`

Now open `lcov/index.html` in a browser.

Unit testing
------------

We use the Boost unit test framework for unit testing.  Most unit
tests reside in the `chain_test` build target.

Witness node
------------

The role of the witness node is to broadcast transactions, download blocks, and optionally sign them.

```
./witness_node --rpc-endpoint "127.0.0.1:8090" --enable-stale-production -w \""1.6.0"\" \""1.6.1"\" \""1.6.2"\" \""1.6.3"\" \""1.6.4"\"
```

Running specific tests
----------------------

- `tests/chain_tests -t block_tests/name_of_test`

Using the API
-------------

We provide several different API's.  Each API has its own ID.
When running `witness_node`, initially two API's are available:
API 0 provides read-only access to the database, while API 1 is
used to login and gain access to additional, restricted API's.

Here is an example using `wscat` package from `npm` for websockets:

    $ npm install -g wscat
    $ wscat -c ws://127.0.0.1:8090
    > {"id":1, "method":"call", "params":[0,"get_accounts",[["1.2.0"]]]}
    < {"id":1,"result":[{"id":"1.2.0","annotations":[],"membership_expiration_date":"1969-12-31T23:59:59","registrar":"1.2.0","referrer":"1.2.0","lifetime_referrer":"1.2.0","network_fee_percentage":2000,"lifetime_referrer_fee_percentage":8000,"referrer_rewards_percentage":0,"name":"committee-account","owner":{"weight_threshold":1,"account_auths":[],"key_auths":[],"address_auths":[]},"active":{"weight_threshold":6,"account_auths":[["1.2.5",1],["1.2.6",1],["1.2.7",1],["1.2.8",1],["1.2.9",1],["1.2.10",1],["1.2.11",1],["1.2.12",1],["1.2.13",1],["1.2.14",1]],"key_auths":[],"address_auths":[]},"options":{"memo_key":"GPH1111111111111111111111111111111114T1Anm","voting_account":"1.2.0","num_witness":0,"num_committee":0,"votes":[],"extensions":[]},"statistics":"2.7.0","whitelisting_accounts":[],"blacklisting_accounts":[]}]}

We can do the same thing using an HTTP client such as `curl` for API's which do not require login or other session state:

    $ curl --data '{"jsonrpc": "2.0", "method": "call", "params": [0, "get_accounts", [["1.2.0"]]], "id": 1}' http://127.0.0.1:8090/rpc
    {"id":1,"result":[{"id":"1.2.0","annotations":[],"membership_expiration_date":"1969-12-31T23:59:59","registrar":"1.2.0","referrer":"1.2.0","lifetime_referrer":"1.2.0","network_fee_percentage":2000,"lifetime_referrer_fee_percentage":8000,"referrer_rewards_percentage":0,"name":"committee-account","owner":{"weight_threshold":1,"account_auths":[],"key_auths":[],"address_auths":[]},"active":{"weight_threshold":6,"account_auths":[["1.2.5",1],["1.2.6",1],["1.2.7",1],["1.2.8",1],["1.2.9",1],["1.2.10",1],["1.2.11",1],["1.2.12",1],["1.2.13",1],["1.2.14",1]],"key_auths":[],"address_auths":[]},"options":{"memo_key":"GPH1111111111111111111111111111111114T1Anm","voting_account":"1.2.0","num_witness":0,"num_committee":0,"votes":[],"extensions":[]},"statistics":"2.7.0","whitelisting_accounts":[],"blacklisting_accounts":[]}]}

API 0 is accessible using regular JSON-RPC:

    $ curl --data '{"jsonrpc": "2.0", "method": "get_accounts", "params": [["1.2.0"]], "id": 1}' http://127.0.0.1:8090/rpc

Accessing restricted API's
--------------------------

You can restrict API's to particular users by specifying an `apiaccess` file in `config.ini`.  Here is an example `apiaccess` file which allows
user `bytemaster` with password `supersecret` to access four different API's:

    {
       "permission_map" :
       [
          [
             "bytemaster",
             {
                "password_hash_b64" : "9e9GF7ooXVb9k4BoSfNIPTelXeGOZ5DrgOYMj94elaY=",
                "password_salt_b64" : "INDdM6iCi/8=",
                "allowed_apis" : ["database_api", "network_broadcast_api", "history_api", "network_node_api"]
             }
          ]
       ]
    }

Passwords are stored in `base64` as as salted `sha256` hashes.  A simple Python script, `saltpass.py` is avaliable to obtain hash and salt values from a password.
A single asterisk `"*"` may be specified as username or password hash to accept any value.

With the above configuration, here is an example of how to call `add_node` from the `network_node` API:

    {"id":1, "method":"call", "params":[1,"login",["bytemaster", "supersecret"]]}
    {"id":2, "method":"call", "params":[1,"network_node",[]]}
    {"id":3, "method":"call", "params":[2,"add_node",["127.0.0.1:9090"]]}

Note, the call to `network_node` is necessary to obtain the correct API identifier for the network API.  It is not guaranteed that the network API identifier will always be `2`.

Since the `network_node` API requires login, it is only accessible over the websocket RPC.  Our `doxygen` documentation contains the most up-to-date information
about API's for the [witness node](https://bitshares.github.io/doxygen/namespacegraphene_1_1app.html) and the
[wallet](https://bitshares.github.io/doxygen/classgraphene_1_1wallet_1_1wallet__api.html).
If you want information which is not available from an API, it might be available
from the [database](https://bitshares.github.io/doxygen/classgraphene_1_1chain_1_1database.html);
it is fairly simple to write API methods to expose database methods.

Running private testnet
-----------------------

Normally `witness_node` assumes it won't be producing blocks from
genesis, or against very old chain state.  We need to get `witness_node`
to discard this assumption if we actually want to start a new chain,
so we will need to specify in `config.ini`:

    enable-stale-production = true

We also need to specify which witnesses will produce blocks locally;
`witness_node` does not assume that it should produce blocks for a given
witness just because it has the correct private key to do so.  There are
ten witnesses at genesis of the testnet, block production can be
enabled for all of them by specifying multiple times in `config.ini`:

    witness-id = "1.6.0"
    witness-id = "1.6.1"
    witness-id = "1.6.2"
    witness-id = "1.6.3"
    witness-id = "1.6.4"
    witness-id = "1.6.5"
    witness-id = "1.6.6"
    witness-id = "1.6.7"
    witness-id = "1.6.8"
    witness-id = "1.6.9"

Questions
---------

- Is there a way to generate help with parameter names and method descriptions?

    Yes. Documentation of the code base, including APIs, can be generated using Doxygen. Simply run `doxygen` in this directory.

    If both Doxygen and perl are available in your build environment, the CLI wallet's `help` and `gethelp`
    commands will display help generated from the doxygen documentation.

    If your CLI wallet's `help` command displays descriptions without parameter names like
        `signed_transaction transfer(string, string, string, string, string, bool)`
    it means CMake was unable to find Doxygen or perl during configuration.  If found, the
    output should look like this:
        `signed_transaction transfer(string from, string to, string amount, string asset_symbol, string memo, bool broadcast)`

- Is there a way to allow external program to drive `cli_wallet` via websocket, JSONRPC, or HTTP?

    Yes. External programs may connect to the CLI wallet and make its calls over a websockets API. To do this, run the wallet in
    server mode, i.e. `cli_wallet -s "127.0.0.1:9999"` and then have the external program connect to it over the specified port
    (in this example, port 9999).

- Is there a way to access methods which require login over HTTP?

    No.  Login is inherently a stateful process (logging in changes what the server will do for certain requests, that's kind
    of the point of having it).  If you need to track state across HTTP RPC calls, you must maintain a session across multiple
    connections.  This is a famous source of security vulnerabilities for HTTP applications.  Additionally, HTTP is not really
    designed for "server push" notifications, and we would have to figure out a way to queue notifications for a polling client.

    Websockets solves all these problems.  If you need to access Graphene's stateful methods, you need to use Websockets.

- What is the meaning of `a.b.c` numbers?

    The first number specifies the *space*.  Space 1 is for protocol objects, 2 is for implementation objects.
    Protocol space objects can appear on the wire, for example in the binary form of transactions.
    Implementation space objects cannot appear on the wire and solely exist for implementation
    purposes, such as optimization or internal bookkeeping.

    The second number specifies the *type*.  The type of the object determines what fields it has.  For a
    complete list of type ID's, see `enum object_type` and `enum impl_object_type` in
    [types.hpp](https://github.com/cryptonomex/graphene/blob/master/libraries/chain/include/graphene/chain/types.hpp).

    The third number specifies the *instance*.  The instance of the object is different for each individual
    object.

- The answer to the previous question was really confusing.  Can you make it clearer?

    All account ID's are of the form `1.2.x`.  If you were the 9735th account to be registered,
    your account's ID will be `1.2.9735`.  Account `0` is special (it's the "committee account,"
    which is controlled by the committee members and has a few abilities and restrictions other accounts
    do not).

    All asset ID's are of the form `1.3.x`.  If you were the 29th asset to be registered,
    your asset's ID will be `1.3.29`.  Asset `0` is special (it's BTS, which is considered the "core asset").

    The first and second number together identify the kind of thing you're talking about (`1.2` for accounts,
    `1.3` for assets).  The third number identifies the particular thing.
