"""Set project defaults and add fixtures for pytest."""

import os
import tempfile
import pytest
import inspect
from pathlib import Path

from jwst.associations import AssociationRegistry, AssociationPool
from jwst.associations.tests.helpers import t_path


@pytest.fixture
def jail_environ():
    """Lock changes to the environment."""
    original = os.environ.copy()
    try:
        yield
    finally:
        os.environ = original


@pytest.fixture(scope="session")
def full_pool_rules(request):
    """
    Set up the full example pool and registry.

    Returns
    -------
    pool: AssociationPool
        The full example pool as read from data/mega_pool.csv.
    rules: AssociationRegistry
        The registry of available associations.
    pool_fname: str
        The full test path to mega_pool.csv.
    """
    pool_fname = t_path("data/mega_pool.csv")
    pool = AssociationPool.read(pool_fname)
    rules = AssociationRegistry()

    return pool, rules, pool_fname


@pytest.fixture
def mk_tmp_dirs():
    """Create a set of temporary directories and change to one of them."""
    tmp_current_path = tempfile.mkdtemp()
    tmp_data_path = tempfile.mkdtemp()
    tmp_config_path = tempfile.mkdtemp()

    old_path = os.getcwd()
    try:
        os.chdir(tmp_current_path)
        yield (tmp_current_path, tmp_data_path, tmp_config_path)
    finally:
        os.chdir(old_path)


@pytest.fixture
def slow(request):
    """
    Set up slow fixture for tests to identify if --slow has been specified.

    Returns
    -------
    bool
        True if --slow has been specified, False otherwise.
    """
    return request.config.getoption("--slow")


@pytest.fixture(scope="module")
def tmp_cwd_module(request, tmp_path_factory):
    """
    Set up fixture to run test in a pristine temporary working directory, scoped to module.

    This allows a test using this fixture to produce files in a
    temporary directory, and then have the tests access them.

    Yields
    ------
    tmp_path
        The temporary directory path.
    """
    old_dir = os.getcwd()
    path = request.module.__name__.split(".")[-1]
    if request._parent_request.fixturename is not None:
        path = path + "_" + request._parent_request.fixturename
    newpath = tmp_path_factory.mktemp(path)
    os.chdir(str(newpath))
    yield newpath
    os.chdir(old_dir)


@pytest.fixture
def tmp_cwd(tmp_path):
    """Perform test in a pristine temporary working directory, scoped to function."""
    old_dir = Path.cwd()
    os.chdir(tmp_path)
    try:
        yield tmp_path
    finally:
        os.chdir(old_dir)


@pytest.hookimpl(trylast=True)
def pytest_configure(config):
    """Add the test description plugin to the pytest configuration."""
    terminal_reporter = config.pluginmanager.getplugin("terminalreporter")
    config.pluginmanager.register(TestDescriptionPlugin(terminal_reporter), "testdescription")


class TestDescriptionPlugin:
    """
    Pytest plugin to print the test docstring when `pytest -vv` is used.

    This plug-in was added to support JWST instrument team testing and
    reporting for the JWST calibration pipeline.
    """

    def __init__(self, terminal_reporter):
        self.terminal_reporter = terminal_reporter
        self.desc = None

    def pytest_runtest_protocol(self, item):
        """Get the docstring for the test."""
        try:
            self.desc = inspect.getdoc(item.obj)
        except AttributeError:
            self.desc = None

    @pytest.hookimpl(hookwrapper=True, tryfirst=True)
    def pytest_runtest_logstart(self, nodeid, location):
        """Print the test docstring when `pytest -vv` is used."""
        # When run as `pytest` or `pytest -v`, no change in behavior
        if self.terminal_reporter.verbosity <= 1:
            yield
        # When run as `pytest -vv`, `pytest -vvv`, etc, print the test docstring
        else:
            self.terminal_reporter.write("\n")
            yield
            if self.desc:
                self.terminal_reporter.write(f"\n{self.desc} ")
