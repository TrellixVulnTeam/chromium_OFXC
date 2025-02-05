// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Used to create fake data for both passwords and autofill.
 * These sections are related, so it made sense to share this.
 */

function FakeDataMaker() {}

/**
 * Creates a single item for the list of passwords.
 * @param {string|undefined} url
 * @param {string|undefined} username
 * @param {number|undefined} passwordLength
 * @return {chrome.passwordsPrivate.PasswordUiEntry}
 */
FakeDataMaker.passwordEntry = function(url, username, passwordLength) {
  // Generate fake data if param is undefined.
  url = url || FakeDataMaker.patternMaker_('www.xxxxxx.com', 16);
  username = username || FakeDataMaker.patternMaker_('user_xxxxx', 16);
  passwordLength = passwordLength || Math.floor(Math.random() * 15) + 3;

  return {
    loginPair: {
      originUrl: url,
      username: username,
    },
    linkUrl: 'http://' + url + '/login',
    numCharactersInPassword: passwordLength,
  };
};

/**
 * Creates a single item for the list of password exceptions.
 * @param {string|undefined} url
 * @return {chrome.passwordsPrivate.ExceptionPair}
 */
FakeDataMaker.exceptionEntry = function(url) {
  url = url || FakeDataMaker.patternMaker_('www.xxxxxx.com', 16);
  return {
    exceptionUrl: url,
    linkUrl: 'http://' + url + '/login',
  };
};

/**
 * Creates a new fake address entry for testing.
 * @return {!chrome.autofillPrivate.AddressEntry}
 */
FakeDataMaker.emptyAddressEntry = function() {
  return {};
}

/**
 * Creates a fake address entry for testing.
 * @return {!chrome.autofillPrivate.AddressEntry}
 */
FakeDataMaker.addressEntry = function() {
  var ret = {};
  ret.guid = FakeDataMaker.makeGuid_();
  ret.fullNames = ['John Doe'];
  ret.companyName = 'Google';
  ret.addressLines = FakeDataMaker.patternMaker_('xxxx Main St', 10);
  ret.addressLevel1 = 'CA';
  ret.addressLevel2 = 'Venice';
  ret.postalCode = FakeDataMaker.patternMaker_('xxxxx', 10);
  ret.countryCode = 'US';
  ret.phoneNumbers = [FakeDataMaker.patternMaker_('(xxx) xxx-xxxx', 10)];
  ret.emailAddresses = [FakeDataMaker.patternMaker_('userxxxx@gmail.com', 16)];
  ret.languageCode = 'EN-US';
  ret.metadata = {isLocal: true};
  ret.metadata.summaryLabel = ret.fullNames[0];
  ret.metadata.summarySublabel = ', ' + ret.addressLines;
  return ret;
};

/**
 * Creates a new empty credit card entry for testing.
 * @return {!chrome.autofillPrivate.CreditCardEntry}
 */
FakeDataMaker.emptyCreditCardEntry = function() {
  var now = new Date();
  var expirationMonth = now.getMonth() + 1;
  var ret = {};
  ret.expirationMonth = expirationMonth.toString()
  ret.expirationYear = now.getFullYear().toString();
  return ret;
};

/**
 * Creates a new random credit card entry for testing.
 * @return {!chrome.autofillPrivate.CreditCardEntry}
 */
FakeDataMaker.creditCardEntry = function() {
  var ret = {};
  ret.guid = FakeDataMaker.makeGuid_();
  ret.name = 'Jane Doe';
  ret.cardNumber = FakeDataMaker.patternMaker_('xxxx xxxx xxxx xxxx', 10);
  ret.expirationMonth = Math.ceil(Math.random() * 11).toString();
  ret.expirationYear = (2016 + Math.floor(Math.random() * 5)).toString();
  ret.metadata = {isLocal: true};
  var cards = ['Visa', 'Mastercard', 'Discover', 'Card'];
  var card = cards[Math.floor(Math.random() * cards.length)];
  ret.metadata.summaryLabel = card + ' ' + '****' + ret.cardNumber.substr(-4);
  return ret;
};

/**
 * Creates a new random GUID for testing.
 * @return {string}
 * @private
 */
FakeDataMaker.makeGuid_ = function() {
  return FakeDataMaker.patternMaker_(
      'xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx', 16);
};

/**
 * Replaces any 'x' in a string with a random number of the base.
 * @param {string} pattern The pattern that should be used as an input.
 * @param {number} base The number base. ie: 16 for hex or 10 for decimal.
 * @return {string}
 * @private
 */
FakeDataMaker.patternMaker_ = function(pattern, base) {
  return pattern.replace(/x/g, function() {
    return Math.floor(Math.random() * base).toString(base);
  });
};

/** @constructor */
function PasswordManagerExpectations() {
  this.requested = {
    passwords: 0,
    exceptions: 0,
    plaintextPassword: 0,
  };

  this.removed = {
    passwords: 0,
    exceptions: 0,
  };

  this.listening = {
    passwords: 0,
    exceptions: 0,
  };
}

/**
 * Test implementation
 * @implements {PasswordManager}
 * @constructor
 */
function TestPasswordManager() {
  this.actual_ = new PasswordManagerExpectations();

  // Set these to have non-empty data.
  this.data = {
    passwords: [],
    exceptions: [],
  };

  // Holds the last callbacks so they can be called when needed/
  this.lastCallback = {
    addSavedPasswordListChangedListener: null,
    addExceptionListChangedListener: null,
    getPlaintextPassword: null,
  };
}

TestPasswordManager.prototype = {
  /** @override */
  addSavedPasswordListChangedListener: function(listener) {
    this.actual_.listening.passwords++;
    this.lastCallback.addSavedPasswordListChangedListener = listener;
  },

  /** @override */
  removeSavedPasswordListChangedListener: function(listener) {
    this.actual_.listening.passwords--;
  },

  /** @override */
  getSavedPasswordList: function(callback) {
    this.actual_.requested.passwords++;
    callback(this.data.passwords);
  },

  /** @override */
  removeSavedPassword: function(loginPair) {
    this.actual_.removed.passwords++;

    if (this.onRemoveSavedPassword)
      this.onRemoveSavedPassword(loginPair);
  },

  /** @override */
  addExceptionListChangedListener: function(listener) {
    this.actual_.listening.exceptions++;
    this.lastCallback.addExceptionListChangedListener = listener;
  },

  /** @override */
  removeExceptionListChangedListener: function(listener) {
    this.actual_.listening.exceptions--;
  },

  /** @override */
  getExceptionList: function(callback) {
    this.actual_.requested.exceptions++;
    callback(this.data.exceptions);
  },

  /** @override */
  removeException: function(exception) {
    this.actual_.removed.exceptions++;

    if (this.onRemoveException)
      this.onRemoveException(exception);
  },

  /** @override */
  getPlaintextPassword: function(loginPair, callback) {
    this.actual_.requested.plaintextPassword++;
    this.lastCallback.getPlaintextPassword = callback;
  },

  /**
   * Verifies expectations.
   * @param {!PasswordManagerExpectations} expected
   */
  assertExpectations: function(expected) {
    var actual = this.actual_;

    assertEquals(expected.requested.passwords, actual.requested.passwords);
    assertEquals(expected.requested.exceptions, actual.requested.exceptions);
    assertEquals(expected.requested.plaintextPassword,
                 actual.requested.plaintextPassword);

    assertEquals(expected.removed.passwords, actual.removed.passwords);
    assertEquals(expected.removed.exceptions, actual.removed.exceptions);

    assertEquals(expected.listening.passwords, actual.listening.passwords);
    assertEquals(expected.listening.exceptions, actual.listening.exceptions);
  },
};

/** @constructor */
function AutofillManagerExpectations() {
  this.requested = {
    addresses: 0,
    creditCards: 0,
  };

  this.listening = {
    addresses: 0,
    creditCards: 0,
  };
}

/**
 * Test implementation
 * @implements {AutofillManager}
 * @constructor
 */
function TestAutofillManager() {
  this.actual_ = new AutofillManagerExpectations();

  // Set these to have non-empty data.
  this.data = {
    addresses: [],
    creditCards: [],
  };

  // Holds the last callbacks so they can be called when needed.
  this.lastCallback = {
    addAddressListChangedListener: null,
    addCreditCardListChangedListener: null,
  };
}

TestAutofillManager.prototype = {
  /** @override */
  addAddressListChangedListener: function(listener) {
    this.actual_.listening.addresses++;
    this.lastCallback.addAddressListChangedListener = listener;
  },

  /** @override */
  removeAddressListChangedListener: function(listener) {
    this.actual_.listening.addresses--;
  },

  /** @override */
  getAddressList: function(callback) {
    this.actual_.requested.addresses++;
    callback(this.data.addresses);
  },

  /** @override */
  addCreditCardListChangedListener: function(listener) {
    this.actual_.listening.creditCards++;
    this.lastCallback.addCreditCardListChangedListener = listener;
  },

  /** @override */
  removeCreditCardListChangedListener: function(listener) {
    this.actual_.listening.creditCards--;
  },

  /** @override */
  getCreditCardList: function(callback) {
    this.actual_.requested.creditCards++;
    callback(this.data.creditCards);
  },

  /**
   * Verifies expectations.
   * @param {!AutofillManagerExpectations} expected
   */
  assertExpectations: function(expected) {
    var actual = this.actual_;

    assertEquals(expected.requested.addresses, actual.requested.addresses);
    assertEquals(expected.requested.creditCards, actual.requested.creditCards);

    assertEquals(expected.listening.addresses, actual.listening.addresses);
    assertEquals(expected.listening.creditCards, actual.listening.creditCards);
  },
};
