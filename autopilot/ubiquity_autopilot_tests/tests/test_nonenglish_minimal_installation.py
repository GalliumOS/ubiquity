# Testing Default Install for the Ubiquity Installer
# Author: Heber Parrucci <heber.parrucci@canonical.com>
# Copyright (C) 2018
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
from ubiquity_autopilot_tests.tests import UbiquityAutopilotTestCase
from testtools.matchers import Equals
from autopilot.matchers import Eventually
from ubiquity_autopilot_tests.emulators import gtktoplevel


class NonEnglishMinimalInstallationTestCase(UbiquityAutopilotTestCase):
    """
    Test choosing Non English and minimal installation option
    """

    def test_default_install(self, ):
        # first check we have an emulator instance
        flavor = self.get_distribution()
        self.assertIsInstance(self.main_window, gtktoplevel.GtkWindow)
        self.assertThat(self.main_window.visible, Eventually(Equals(True)))
        self.welcome_page_tests()
        self.go_to_next_page()
        self.keyboard_layout_page_tests()
        self.go_to_next_page()
        self.preparing_page_tests(minimal_installation=True)
        self.go_to_next_page()
        if flavor == 'Edubuntu':
            self.edubuntu_addon_window_tests(gnome=True)
            self.go_to_next_page()
            self.edubuntu_packages_window_tests()
            self.go_to_next_page()
        self.installation_type_page_tests(default=True)
        self.go_to_next_page(wait=True)
        self.location_page_tests()
        self.go_to_next_page()
        self.user_info_page_tests('Autopilot', 'password')
        self.go_to_progress_page()

        self.progress_page_tests()
        self.assertThat(lambda: self.app.select_single(
            'GtkDialog',
            name='finished_dialog').visible,
            Eventually(Equals(True), timeout=180))
        # XXX: Uncomment if you want to restart after install complete
        # we need to sleep here to wait for dialog to fully load. It appears
        # in dbus before its actually visible. As the test has already passed
        # this doesn't affect outcome
        # time.sleep(5)
        #
        # self.keyboard.press_and_release('Enter')
