// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function makeURL(toolchain, config) {
    return 'index.html?tc=' + toolchain + '&config=' + config;
}

function createWindow(url) {
    console.log('loading ' + url);
    chrome.app.window.create(url, {
        width: 1024,
        height: 800,
        frame: 'none'
    });
}

function onLaunched(launchData) {
    createWindow('index.html');
}

chrome.app.runtime.onLaunched.addListener(onLaunched);
