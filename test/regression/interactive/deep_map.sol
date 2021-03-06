// RUN: %solc %s --c-model --lockstep-time=off --reps=3 --output-dir=%t
// RUN: cd %t
// RUN: cmake -DSEA_PATH=%seapath
// RUN: make icmodel
// RUN: echo 0 0 2 1 0 0 0 2 2 3 10 1 0 0 1 2 2 3 10 1 0 0 0 3 2 3 10 1 0 0 1 3 2 3 10 1 0 0 1 5 2 3 10 | ./icmodel --return-0 --count-transactions 2>&1 | OutputCheck %s --comment=//
// CHECK: assert
// CHECK: Transaction Count: 5

/**
 * Ensures map nesting can work.
 */

contract DeepMap {
    mapping(address => mapping(address => mapping(address => int))) db;

    function write(address a, address b, int val) public {
        db[msg.sender][a][b] = val;
    }

    function check(address a, address b, int val) public view {
        assert(db[msg.sender][a][b] == val);
    }
}
