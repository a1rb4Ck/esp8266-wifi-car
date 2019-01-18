/*

ESP8266 file system builder

Copyright (C) 2016-2018 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

https://github.com/xoseperez/espurna/blob/dev/code/gulpfile.js
http://tinkerman.cat/optimizing-files-for-spiffs-with-gulp/

*/

/*eslint quotes: ['error', 'single']*/
/*eslint-env es6*/

// -----------------------------------------------------------------------------
// Dependencies
// -----------------------------------------------------------------------------

const gulp = require('gulp');

const htmlmin = require('gulp-htmlmin');
const inline = require('gulp-inline');
const inlineImages = require('gulp-css-base64');
const favicon = require('gulp-base64-favicon');
const crass = require('gulp-crass');
const htmllint = require('gulp-htmllint');

const rename = require('gulp-rename');
const uglify = require('gulp-uglify')
const gzip = require('gulp-gzip');
const path = require('path');

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

const htmlFolder = '../html/';
const dataFolder = '../data/';

// -----------------------------------------------------------------------------
// Methods
// -----------------------------------------------------------------------------

var htmllintReporter = function(filepath, issues) {
    if (issues.length > 0) {
        issues.forEach(function (issue) {
            console.info(
                '[gulp-htmllint] ' +
                filepath + ' [' +
                issue.line + ',' +
                issue.column + ']: ' +
                '(' + issue.code + ') ' +
                issue.msg
            );
        });
        process.exitCode = 1;
    }
};

var buildWebUI = function() {
    return gulp.src(htmlFolder + '*.html').
        pipe(htmllint({
            'failOnError': false,
            'rules': {
                'id-class-style': false,
                'label-req-for': false,
            }
        }, htmllintReporter)).
        // pipe(favicon()).
        pipe(inline({
            base: htmlFolder,
            js: uglify,
            css: [crass, inlineImages],
            disabledTypes: ['svg', 'img']
        })).
        pipe(htmlmin({
            collapseWhitespace: true,
            removeComments: true,
            minifyCSS: true,
            minifyJS: true
        })).
        // pipe(gzip()).
        pipe(rename('index.html')).
        pipe(gulp.dest(dataFolder));
};

// -----------------------------------------------------------------------------
// Tasks
// -----------------------------------------------------------------------------

gulp.task('index', function() {
    return buildWebUI('small');
});

gulp.task('default', gulp.task('index'));