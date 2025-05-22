# leetcode_validator/main.py

import ast
import importlib.util
import os
import sys
import time
import copy
from ast import literal_eval

ALLOWED_NODES = {
    ast.Module, ast.FunctionDef, ast.arguments, ast.arg,
    ast.Assign, ast.AugAssign, ast.Return,
    ast.For, ast.While, ast.If, ast.Break, ast.Continue,
    ast.Expr,
    ast.Name, ast.Load, ast.Store,
    ast.Constant, ast.BinOp, ast.UnaryOp, ast.BoolOp, ast.Compare,
    ast.Subscript, ast.List, ast.Tuple,
    ast.Call,
    ast.Add, ast.Sub, ast.Mult, ast.Div, ast.FloorDiv, ast.Mod, ast.Pow,
    ast.And, ast.Or, ast.Not,
    ast.Eq, ast.NotEq, ast.Lt, ast.LtE, ast.Gt, ast.GtE,
}

SAFE_FUNCTIONS = {"len", "range", "min", "max", "sum", "abs", "enumerate"}

def validate_code(path):
    with open(path, "r") as f:
        source = f.read()

    tree = ast.parse(source)

    # Must be a single top-level function
    if len(tree.body) != 1 or not isinstance(tree.body[0], ast.FunctionDef):
        raise ValueError("Submission must contain exactly one top-level function.")

    for node in ast.walk(tree):
        if type(node) not in ALLOWED_NODES:
            raise ValueError(f"Disallowed AST node: {type(node).__name__}")

        # Disallow all imports
        if isinstance(node, (ast.Import, ast.ImportFrom)):
            raise ValueError("Imports are not allowed.")

        # Allow safe function calls only
        if isinstance(node, ast.Call):
            if not isinstance(node.func, ast.Name) or node.func.id not in SAFE_FUNCTIONS:
                raise ValueError(f"Function call to '{getattr(node.func, 'id', '?')}' is not allowed.")

    return tree.body[0].name

def load_player_function(path):
    """
    Validate and load the player's submitted function from a file.
    Returns the function object.
    """
    func_name = validate_code(path)
    spec = importlib.util.spec_from_file_location("player_code", path)
    module = importlib.util.module_from_spec(spec)
    sys.modules["player_code"] = module
    spec.loader.exec_module(module)

    if not hasattr(module, func_name):
        raise RuntimeError("Function not found after module load.")

    func = getattr(module, func_name)
    if not callable(func):
        raise RuntimeError("Submitted object is not callable.")
    return func


def run_single_test(func, test_input):
    """
    Run a single test case.
    Returns a tuple:
      (test_input, player_output, expected_output, time_taken, correct)
    Expected output is computed using sorted() on the test input.
    """
    start_time = time.perf_counter()
    player_output = func(test_input)
    elapsed = time.perf_counter() - start_time

    # Compute expected result using built-in sorted
    expected = sorted(test_input)
    correct = (player_output == expected)
    return (test_input, player_output, expected, elapsed, correct)


def run_test_cases(player_path, test_cases_path):
    """
    Loads the player's function and runs it against test cases provided in a file.
    Each line should contain a tuple: (input_list, expected_output_list)
    Returns and prints the result for each test case.
    """
    func = load_player_function(player_path)

    test_results = []
    with open(test_cases_path, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                # Parse each line as a tuple of (input, expected)
                test_input, expected = literal_eval(line)
                if not isinstance(test_input, list) or not isinstance(expected, list):
                    raise ValueError("Each test case must be a tuple of two lists.")
            except Exception as e:
                print(f"Error parsing test case: {line}\n{e}")
                continue

            start_time = time.perf_counter()
            try:
                test_input_copy = copy.deepcopy(test_input)
                player_output = func(test_input_copy)
            except Exception as e:
                player_output = f"Runtime error: {e}"
                correct = False
            else:
                correct = player_output == expected
            elapsed = time.perf_counter() - start_time

            test_results.append((test_input, player_output, expected, elapsed, correct))

    print("Test Results:")

    DEBUG_MODE = os.environ.get("DEBUG","false") == "true"

    if DEBUG_MODE:
        print("Case\tCorrect\tTime (sec)\tPlayer Output\tExpected")
        i = 1
        for inp, outp, expected, t, correct in test_results:
            print(f"{i}\t{correct}\t{t:.6f}\t{outp}\t{expected}")
            i+=1
    else:
        print("Case\tCorrect\tTime (sec)")
        i = 1
        for inp, outp, _, t, correct in test_results:
            print(f"{i}\t{correct}\t{t:.6f}")
            i+=1



def main():
    """
    Main entry point.
    Usage:
      python -m leetcode_validator.main path/to/main.py [path/to/testcases.txt]
    If a test case file is provided, the function is run against each test case.
    Otherwise, a single default test input is used.
    """
    if len(sys.argv) < 2:
        print("Usage: python -m leetcode_validator.main path/to/main.py [path/to/testcases.txt]")
        sys.exit(1)

    player_path = sys.argv[1]
    debug = os.environ.get("DEBUG")
    if debug == "true":
        print("[DEBUG] Running in debug mode.")

    if len(sys.argv) == 3:
        test_cases_path = sys.argv[2]
        run_test_cases(player_path, test_cases_path)
    else:
        # Use a default test case if no test file is provided
        func = load_player_function(player_path)
        default_input = [5, 2, 1, 9, 5, 6]
        result = run_single_test(func, default_input)
        print("Test Result:")
        print("Input:", result[0])
        print("Player Output:", result[1])
        print("Expected:", result[2])
        print("Time Taken (sec):", f"{result[3]:.6f}")
        print("Correct:", result[4])


if __name__ == "__main__":
    main()
