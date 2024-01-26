var mysql = require('mysql');
var connection = mysql.createConnection({
    host: 'localhost',
    user: 'root',
    password: '',
    database: 'smart_parking'
});

// var connect = function() {
connection.connect(function(err) {
    if (!err) {
        console.log("Database is connected!");
    } else {
        console.log("Database is error!");
    }
});
// };


var closeDB = function() {
    connection.end(function(err) {
        if (!err) console.console.log("Closed database");
    });
};
const queryAsync = (sql) => {
    return new Promise((resolve, reject) => {
        connection.query(sql, (err, result) => {
            if (err) reject(err)
            else resolve(result)
        })
    })
}
exports.getAllUser = () => queryAsync("SELECT * FROM `users`")
    // exports.getByLicensePlate = function(license_plate, UID, callbackQuery) {
    //     connect();
    //     connection.query("SELECT * FROM `users` WHERE `license_plate` = '" + license_plate + "' AND `UID` = '" + UID + "'", function(err, results, fields) {
    //         if (!err) {
    //             callbackQuery(results);
    //         } else {
    //             console.log(err);
    //         }
    //     });
    // };
exports.getByLicensePlate = (UID) => {
    return queryAsync(`SELECT * FROM \`users\` WHERE \`UID\` = '${UID}'`)
}

exports.insertData = function(username, license_plate, email, phone_number, callbackInsert) {
    connect();
    connection.query("INSERT INTO `users` (`id`, `full_name`, `birthday`, `license_plate`, `email`, `phone_number`, `expiration_date`)" +
        " VALUES (NULL, '" + username + "', '2021-10-01', '" + license_plate + "', '" + email + "', '" + phone_number + "', '2021-10-05');",
        function(err, results, fields) {
            if (!err) {
                callbackInsert(results);
            } else {
                console.log(err);
            }
        });
};