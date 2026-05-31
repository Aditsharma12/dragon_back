#include <drogon/drogon.h>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cstdlib>

// ── shared HTML shell ───────────────────────────────────────────────────────
std::string page(const std::string& body) {
    return R"html(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <title>C++ Calculator</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;600;700&display=swap');
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      min-height: 100vh;
      display: flex; align-items: center; justify-content: center;
      background: linear-gradient(135deg, #0f0c29, #302b63, #24243e);
      font-family: 'Inter', sans-serif;
      color: #e2e8f0;
    }
    .card {
      background: rgba(255,255,255,0.07);
      backdrop-filter: blur(18px);
      border: 1px solid rgba(255,255,255,0.12);
      border-radius: 20px;
      padding: 2.5rem 3rem;
      width: 100%;
      max-width: 460px;
      box-shadow: 0 25px 60px rgba(0,0,0,0.5);
    }
    h1 {
      font-size: 1.6rem; font-weight: 700;
      text-align: center; margin-bottom: 0.3rem;
      background: linear-gradient(90deg, #a78bfa, #60a5fa);
      -webkit-background-clip: text; -webkit-text-fill-color: transparent;
    }
    p.sub { text-align:center; font-size:.85rem; color:#94a3b8; margin-bottom:2rem; }
    label { display:block; font-size:.8rem; font-weight:600;
            color:#94a3b8; margin-bottom:.4rem; letter-spacing:.05em; }
    input[type=number], select {
      width: 100%; padding: .75rem 1rem;
      background: rgba(255,255,255,0.08);
      border: 1px solid rgba(255,255,255,0.15);
      border-radius: 10px; color: #e2e8f0;
      font-size: 1rem; font-family: inherit;
      transition: border-color .2s, box-shadow .2s;
      margin-bottom: 1.2rem;
    }
    input[type=number]:focus, select:focus {
      outline: none;
      border-color: #a78bfa;
      box-shadow: 0 0 0 3px rgba(167,139,250,.25);
    }
    select option { background: #1e1b4b; }
    .row { display:grid; grid-template-columns:1fr 1fr; gap:1rem; }
    button {
      width: 100%; padding: .85rem;
      background: linear-gradient(135deg, #7c3aed, #2563eb);
      border: none; border-radius: 12px;
      color: #fff; font-size: 1rem; font-weight: 600;
      cursor: pointer; font-family: inherit;
      transition: transform .15s, box-shadow .15s;
    }
    button:hover {
      transform: translateY(-2px);
      box-shadow: 0 8px 25px rgba(124,58,237,.5);
    }
    button:active { transform: translateY(0); }
    .result {
      margin-top: 1.8rem;
      background: rgba(255,255,255,0.05);
      border: 1px solid rgba(255,255,255,0.1);
      border-radius: 14px;
      padding: 1.4rem 1.6rem;
      text-align: center;
    }
    .result .label { font-size:.75rem; color:#94a3b8; letter-spacing:.08em; margin-bottom:.4rem; }
    .result .value {
      font-size: 2.4rem; font-weight: 700;
      background: linear-gradient(90deg, #34d399, #60a5fa);
      -webkit-background-clip: text; -webkit-text-fill-color: transparent;
    }
    .result .expr { font-size:.9rem; color:#64748b; margin-top:.5rem; }
    .error .value { background: linear-gradient(90deg,#f87171,#fb923c); }
    .back { display:block; text-align:center; margin-top:1.2rem;
            font-size:.85rem; color:#7c3aed; text-decoration:none; }
    .back:hover { text-decoration:underline; }
  </style>
</head>
<body>
  <div class="card">
    <h1>⚡ C++ Calculator</h1>
    <p class="sub">Powered by Drogon · computed server-side in C++</p>
)html" + body + R"html(
  </div>
</body>
</html>)html";
}

// ── form HTML ────────────────────────────────────────────────────────────────
const std::string FORM = R"html(
    <form method="POST" action="/calculate">
      <div class="row">
        <div>
          <label for="a">Number A</label>
          <input type="number" id="a" name="a" step="any" placeholder="e.g. 42" required/>
        </div>
        <div>
          <label for="b">Number B</label>
          <input type="number" id="b" name="b" step="any" placeholder="e.g. 7" required/>
        </div>
      </div>
      <label for="op">Operation</label>
      <select id="op" name="op">
        <option value="add">➕  Add  (A + B)</option>
        <option value="sub">➖  Subtract  (A − B)</option>
        <option value="mul">✖️  Multiply  (A × B)</option>
        <option value="div">➗  Divide  (A ÷ B)</option>
        <option value="pow">🔺  Power  (A ^ B)</option>
        <option value="mod">〰️  Modulo  (A % B)</option>
      </select>
      <button type="submit">Calculate →</button>
    </form>
)html";

int main() {
    // Read PORT from environment (Render injects this), fallback to 8080 locally
    const char* portEnv = std::getenv("PORT");
    int port = portEnv ? std::atoi(portEnv) : 8080;

    drogon::app().addListener("0.0.0.0", port);

    // ── GET / — serve the form ───────────────────────────────────────────────
    drogon::app().registerHandler(
        "/",
        [](const drogon::HttpRequestPtr& req,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setContentTypeCode(drogon::CT_TEXT_HTML);
            resp->setBody(page(FORM));
            callback(resp);
        },
        {drogon::Get}
    );

    // ── POST /calculate — compute and return result ──────────────────────────
    drogon::app().registerHandler(
        "/calculate",
        [](const drogon::HttpRequestPtr& req,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {

            auto params = req->getParameters();
            std::string resultHtml;

            try {
                if (params.find("a") == params.end() ||
                    params.find("b") == params.end() ||
                    params.find("op") == params.end())
                    throw std::runtime_error("Missing form fields.");

                double a  = std::stod(params.at("a"));
                double b  = std::stod(params.at("b"));
                std::string op = params.at("op");

                double result = 0.0;
                std::string symbol;

                if      (op == "add") { result = a + b; symbol = "+"; }
                else if (op == "sub") { result = a - b; symbol = "−"; }
                else if (op == "mul") { result = a * b; symbol = "×"; }
                else if (op == "div") {
                    if (b == 0) throw std::runtime_error("Division by zero!");
                    result = a / b; symbol = "÷";
                }
                else if (op == "pow") { result = std::pow(a, b); symbol = "^"; }
                else if (op == "mod") {
                    if (b == 0) throw std::runtime_error("Modulo by zero!");
                    result = std::fmod(a, b); symbol = "%";
                }
                else throw std::runtime_error("Unknown operation.");

                // Format result — strip trailing zeros
                std::ostringstream oss;
                oss << result;
                std::string resStr = oss.str();

                std::ostringstream exprOss;
                exprOss << a << " " << symbol << " " << b << " = " << resStr;

                resultHtml = R"html(
    <div class="result">
      <div class="label">RESULT</div>
      <div class="value">)html" + resStr + R"html(</div>
      <div class="expr">)html" + exprOss.str() + R"html(</div>
    </div>)html" + FORM + R"html(<a class="back" href="/">← New Calculation</a>)html";

            } catch (const std::exception& e) {
                resultHtml = R"html(
    <div class="result error">
      <div class="label">ERROR</div>
      <div class="value" style="font-size:1.3rem">)html" +
                    std::string(e.what()) + R"html(</div>
    </div>)html" + FORM + R"html(<a class="back" href="/">← Try again</a>)html";
            }

            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setContentTypeCode(drogon::CT_TEXT_HTML);
            resp->setBody(page(resultHtml));
            callback(resp);
        },
        {drogon::Post}
    );

    LOG_INFO << "Server running at http://localhost:8080";
    drogon::app().run();
    return 0;
}
