#!/usr/bin/node

'use strict';

if (process.argv.length < 4) {
    console.log('Usage: hex256.js input output');
    process.exit(-1);
}

let fs = require('fs');

let reader = require('readline').createInterface({
    input: fs.createReadStream(process.argv[2])
});

let line_buf = "";
let fd = fs.openSync(process.argv[3], 'w');
if (!fd) {
    console.log(`Cannot open ${process.argv[3]} for writing`);
    process.exit(-1);
}

reader.on('line', function (line) {
    line_buf = line.substr(0, 8) + line_buf;
    if (line_buf.length == 64) {
        line_buf += '\n';
        fs.writeSync(fd, line_buf);
        line_buf = '';
    }
})
.on('close', function () {
    if (line_buf.length) {
        while (line_buf.length < 64) {
            line_buf = '0' + line_buf;
        }
        fs.writeSync(fd, line_buf + '\n');
    }
    fs.close(fd);
    process.exit(-1);
});
