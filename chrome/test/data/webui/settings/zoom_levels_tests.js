// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @fileoverview Suite of tests for zoom-levels. */
cr.define('zoom_levels', function() {
  function registerTests() {
    suite('ZoomLevels', function() {
      /**
       * A zoom levels category created before each test.
       * @type {ZoomLevels}
       */
      var testElement;

      /**
       * The mock proxy object to use during test.
       * @type {TestSiteSettingsPrefsBrowserProxy}
       */
      var browserProxy = null;

      /**
       * An example zoom list.
       * @type {!Array<ZoomLevelEntry>}
       */
      var zoomList = [
          {
            origin: 'http://www.google.com',
            displayName: 'http://www.google.com',
            originForFavicon: 'http://www.google.com',
            setting: '',
            source: '',
            zoom: '125%',
          },
          {
            origin: 'http://www.chromium.org',
            displayName: 'http://www.chromium.org',
            originForFavicon: 'http://www.chromium.org',
            setting: '',
            source: '',
            zoom: '125%',
          },
      ];

      setup(function() {
        browserProxy = new TestSiteSettingsPrefsBrowserProxy();
        settings.SiteSettingsPrefsBrowserProxyImpl.instance_ = browserProxy;
        return initPage();
      });

      teardown(function() {
        testElement.remove();
        testElement = null;
      });

      /** @return {!Promise} */
      function initPage() {
        browserProxy.reset();
        PolymerTest.clearBody();
        testElement = document.createElement('zoom-levels');
        document.body.appendChild(testElement);
        return browserProxy.whenCalled('fetchZoomLevels');
      }

      /**
       * Fetch the remove button from the list.
       * @param {!HTMLElement} listContainer The list to use for the lookup.
       * @param {number} index The index of the child element (which site) to
       *     fetch.
       */
      function getRemoveButton(listContainer, index) {
        return listContainer.children[index].querySelector('paper-icon-button');
      }

      test('empty zoom state', function() {
        var list = testElement.$.list;
        assertTrue(!!list);
        assertEquals(0, list.items.length);
      });

      test('non-empty zoom state', function() {
        browserProxy.setZoomList(zoomList);

        return initPage().then(function() {
          var list = testElement.$.list;
          assertTrue(!!list);
          assertEquals(2, list.items.length);

          var removeButton =
              getRemoveButton(testElement.$.listContainer, 0);
          assert(!!removeButton);
          MockInteractions.tap(removeButton);
          return browserProxy.whenCalled('removeZoomLevel');
        }).then(function(args) {
          assertEquals("http://www.google.com", args[0]);
        });
      });
    });
  }

  return {
    registerTests: registerTests,
  };
});
