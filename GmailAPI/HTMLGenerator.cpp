#include "HTMLGenerator.hpp"

std::string HTMLGenerator::htmlMail(const std::string &content)
{
    std::ostringstream html;
    html << R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8" />
        <style>
            * { box-sizing: border-box; }
            html { font-family: 'Roboto', sans-serif; }
            p, td, th, span, ul { color: #333; font-size: 16px; }
            .main { margin: 0 auto; border: 1px solid #ccc; border-radius: 10px; padding: 6px 30px 30px 30px; width: 700px; }
            .app__name { text-align: center; font-size: 24px; color:rgb(162, 28, 28); font-weight: bold; }
            .app__greeting, .app__desc { text-align: center; }
            .divider { border-bottom: 1px solid #ccc; margin: 20px 0; }
            .response { font-weight: bold; word-break: break-all; }
            table { margin: 0 auto; border-collapse: collapse; overflow: hidden; }
            table td, table th { font-size: 14px; }
            table.left { text-align: left; }
            table.center { text-align: center; }
            td, th { border-top: 1px solid rgba(24, 21, 191, 0.9); padding: 10px 14px; }
            th { background-color: #76dfd8; border-left: 1px solid #c6cbcd; border-right: 1px solid #c6cbcd; }
            td { border-left: 1px solid #c6cbcd; border-right: 1px solid #c6cbcd; }
            tr.first-row { text-align: center; }
            tr.last-row { border-bottom: 1px solid #c6cccde6; }
            tr.odd-row td { background-color: #e6f8f7; }
            .message { margin: 0; }
            .message.bold { font-weight: bold; }
            .message.ok { color: #1e9d95; }
            .message.error { color: red; }
            .ascii { font-family: 'Courier New', monospace; font-size: 16px; margin: 0; margin-left: 40px; white-space: pre-wrap; font-weight: bold; }
        </style>
    </head>
    <body>
        <div class='main'>
            <div class="container">
                <p class="app__name">👻 Remote Control with Email Service 👻</p>
                <p class="app__greeting">Greeting from <span style='font-weight: bold;'>Đàm Tiến Đạt, Huỳnh Cung, Lê Xuân Trí </span></p>
                <p class="app__greeting">Regular Program 2023, University of Science, VNUHCM</p>
                <p class="app__desc">This is final project for <span style='font-weight: bold;'>📚 Computer Networking 🛜</span> course (CSC10008).</p>
                <div class='divider'></div>
            </div>
            <div class="container">
                <p>Response from server: <span class="response" lang="en">)" << content << R"(</span></p>
            </div>
        </div>
    </body>
    </html>
    )";
    return html.str();
}