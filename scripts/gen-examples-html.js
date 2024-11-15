#!/usr/bin/env node

const fs = require("fs/promises");
let file = "";
function println(str = "") {
  print(str + "\n");
}
function print(str = "") {
  file += str;
}

const run = async () => {
  println(`<html>
<head>
<title>Examples</title>
<style>
html, body {
  margin: 0;
  padding: 0;
  width: 100%;
}
.section {
}
.title {
  font-family: sans-serif;
  font-weight: 500;
  font-size: 24px;
  padding: 16px;
}
.items {
  width: 100%;
  display: flex;
  flex-direction: row;
  flex-wrap: wrap;
}
.item {
  width: 20%;
}
.nav {
  display: flex;
  gap: 8px;
}
</style>
</head>
<body>
<div>
<script>
function show(str) {
  const all = document.querySelectorAll('.section');
  all.forEach(el => {el.style.display = 'none'});
  const el = document.querySelector(str);
  el.style.display = 'block';
}
</script>
  `);
  const sections = [
    { id: "iOS-oldarch", dir: "ios/oldarch" },
    { id: "iOS-newarch", dir: "ios/newarch" },
    { id: "Android-oldarch", dir: "android/oldarch" },
    { id: "Android-newarch", dir: "android/newarch" },
  ];
  println("<div class='nav'>");
  sections.forEach(({ id }) => {
    println(`<button onClick="show('#${id}')">${id}</button>`);
  });
  println("</div>");

  for (const { id, dir } of sections) {
    const files = await fs.readdir(dir);
    println(`<div id="${id}" class="section">`);
    println(`<div class="title">${id}</div>`);
    println(`<div class="items">`);
    files.forEach((file) => {
      const name = file.replace(/.\w+$/, "");
      println("<div class='item'>");
      print("<div>");
      print(name);
      print("</div>");
      println(`<img src="${dir}/${file}" alt="${name}" width="100%">`);
      println("</div>");
    });
    println("</div>");
    println("</div>");
  }

  println(`
</div>
</body>
</html>
`);
  console.log(file);
};

run();
