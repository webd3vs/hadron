const undef = "NAME";
const keywords = ["as", "async", "await", "case", "class", "default", "do", "else",
  "false", "for", "from", "func", "fx", "fxn", "if", "import", "new", "return",
  "switch", "true", "while", "null"];

/**
 * @param {string[]} arr
 * @param {number} level
 */
function parse(arr, level) {
  const res = [];
  let letter = "";

  if (arr.length == 1) {
    return arr[0];
  }

  for (i of arr) {
    if (i[level] === undefined) {
      res.push(i);
      continue;
    }
    if (i[level] !== letter) {
      letter = i[level];
      res[letter] = parse(arr.filter((x) => x[level] === letter), level + 1);
    };
  }
  return res;
}

function generate_conditional(tree, indent, level) {
  for (let i in tree) {
    if (typeof tree[i] === "string") {
      const letters = [];
      for (let j = level; j < tree[i].length; j++) {
        letters.push(tree[i][j]);
      }
      letters.push("\\0");

      if (tree[i] == "null") tree[i] = "_NULL";

      str += `${"  ".repeat(indent + 1)}if (${letters.map((x, i) => `k[${level + i}] == ${x ? "'" + x + "'" : x}`).join(" && ")}) return ${tree[i].toUpperCase()};\n`;
    } else {
      str += `${"  ".repeat(indent + 1)}if (k[${level}] == '${i}') {\n`;
      generate_conditional(tree[i], indent + 1, level + 1);
      str += `${"  ".repeat(indent + 1)}}\n`;
    }
  }
  if (level == 0) {
    str += `${"  ".repeat(level + 1)}return ${undef};\n}`;
  }
}

function generate(tree, indent, level) {
  if (level == 0) {
    str += "Type keyword(const char *k) {\n";
    str += `${"  ".repeat(indent + 1)}switch (k[${level}]) {\n`;
    for (let i in tree) {
      str += `${"  ".repeat(indent + 2)}case '${i}':\n`;
      if (typeof tree[i] === "string") {
        const letters = [];
        for (let j = level; j < tree[i].length; j++) {
          letters.push(tree[i][j]);
        }
        letters.push("\\0");

        if (tree[i] == "null") tree[i] = "_NULL";

        str += `${"  ".repeat(indent + 3)}if (${letters.map((x, i) => `k[${level + i}] == ${x ? "'" + x + "'" : x}`).join(" && ")}) return ${tree[i].toUpperCase()};\n`;
      } else {

        generate_conditional(tree[i], indent + 2, level + 1);
      }
      str += `${"  ".repeat(indent + 3)}return NAME;\n`;
    }
    str += `${"  ".repeat(indent + 2)}default:\n`;
    str += `${"  ".repeat(indent + 3)}return ${undef};\n`;
    str += `${"  ".repeat(indent + 1)}}\n`;
  }
}

let str = "";

const tree = parse(keywords, 0);

generate(tree, 0, 0);
console.log(str.replace(/\s+/g, "").replace(/return/g, "return ").replace("Type", "Type ").replace("constchar", "const char "));
