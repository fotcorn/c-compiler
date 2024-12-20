import lit.formats
import os

config.name = 'SelfhostCompiler'
config.test_format = lit.formats.ShTest(True)

# Explicitly set source and execution roots
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.my_test_exec_root)

# Tool substitutions
config.substitutions.append(('%compiler', config.compiler_path))
config.substitutions.append(('%gcc', 'gcc'))

# Features
config.available_features.add('shell')

# Tell lit where to find the test files
config.suffixes = ['.c']  # Look for .c files instead of .test files