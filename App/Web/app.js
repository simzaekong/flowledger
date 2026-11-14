"use strict";

const categories = {
  expense: [
    ["food", "식비"], ["transport", "교통"],
    ["shopping", "쇼핑"], ["hobby", "취미·여가"],
    ["daily", "생활비"], ["school", "교육"],
    ["health", "건강"], ["subscriptions", "구독"],
    ["stock-expense", "주식"], ["other-expense", "기타 지출"]
  ],
  income: [
    ["allowance", "용돈"], ["pay", "급여·알바"],
    ["sale", "판매"], ["stock-income", "주식"],
    ["other-income", "기타 수입"]
  ]
};
const catalog = new Map(Object.values(categories).flat().map(category => [category[0], category]));
const won = new Intl.NumberFormat("ko-KR");
const money = value => won.format(value) + "원";
const get = id => document.getElementById(id);
const storeKey = "flow-ledger-v1";
const bridge = window.webkit?.messageHandlers?.ledger;
const nativeUI = window.webkit?.messageHandlers?.ledgerUI;
document.documentElement.classList.toggle("native-ui", Boolean(nativeUI));
const fields = {
  name: get("entry-name"), amount: get("entry-amount"),
  category: get("entry-category"), date: get("entry-date")
};
const maxAmount = 999_999_999_999;
const today = () => {
  const date = new Date();
  return [date.getFullYear(), date.getMonth() + 1, date.getDate()]
    .map((part, index) => String(part).padStart(index ? 2 : 4, "0")).join("-");
};
let loadError = false;
function load() {
  try {
    const items = bridge ? window.initialLedger : JSON.parse(localStorage.getItem(storeKey) ?? "[]");
    if (!Array.isArray(items)) throw new Error("Invalid ledger");
    return items.map(item => {
      if (!item || !["income", "expense"].includes(item.kind)
          || !categories[item.kind].some(category => category[0] === item.category)
          || typeof item.id !== "string" || typeof item.name !== "string"
          || !Number.isSafeInteger(item.amount) || item.amount <= 0 || item.amount > maxAmount
          || !/^\d{4}-\d{2}-\d{2}$/.test(item.date)) {
        throw new Error("Invalid entry");
      }
      return item;
    });
  } catch {
    loadError = true;
    return [];
  }
}
let records = load();
let time = today();
let year = Number(time.slice(0, 4));
let month = Number(time.slice(5, 7));
let mode = "month";
let kind = "expense";
let selectedDetail = null;
let active;
let totals;
let locked = false;
let customDate = false;
let toastTimer;

function periodLabel() {
  return mode === "year" ? year + "년" : year + "년 " + month + "월";
}
function periodKey() {
  return mode === "year" ? String(year) : year + "-" + String(month).padStart(2, "0");
}
function sum(items) {
  return items.reduce((total, item) => total + BigInt(item.amount), 0n);
}
function defaultDate() {
  const current = today();
  const key = periodKey();
  if (current.startsWith(key)) return current;
  return mode === "month" ? key + "-01" : key + "-01-01";
}
function setPeriodUnit(unit) {
  mode = unit;
  customDate = false;
  draw();
}
function movePeriod(step) {
  if (mode === "year") {
    year = Math.max(1900, Math.min(9999, year + step));
  } else {
    const date = new Date(year, month - 1 + step, 1);
    if (date.getFullYear() < 1900 || date.getFullYear() > 9999) return;
    year = date.getFullYear();
    month = date.getMonth() + 1;
  }
  customDate = false;
  draw();
}
function showError(message) {
  const error = get("form-error");
  error.textContent = message;
  error.hidden = !message;
}
function toast(message) {
  get("feedback").textContent = message;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { get("feedback").textContent = ""; }, 2300);
}
async function save(next) {
  if (locked || loadError) return false;
  locked = true;
  get("add-entry").disabled = true;
  try {
    if (bridge) {
      if (await bridge.postMessage(JSON.stringify(next)) !== true) throw new Error("Save failed");
    } else {
      localStorage.setItem(storeKey, JSON.stringify(next));
    }
    records = next;
    return true;
  } catch {
    return false;
  } finally {
    locked = false;
    get("add-entry").disabled = false;
  }
}
function setKind(value) {
  kind = value;
  get("expense-kind").setAttribute("aria-pressed", String(kind === "expense"));
  get("income-kind").setAttribute("aria-pressed", String(kind === "income"));
  fields.name.placeholder = kind === "expense" ? "지출 이름" : "수입 이름";
  fields.name.setAttribute("aria-label", fields.name.placeholder);
  const placeholder = new Option("카테고리 선택", "", true, true);
  placeholder.disabled = true;
  fields.category.replaceChildren(placeholder,
    ...categories[kind].map(([id, label]) => new Option(label, id)));
}
function draw() {
  active = records.filter(item => item.date.startsWith(periodKey()));
  const income = sum(active.filter(item => item.kind === "income"));
  const expense = sum(active.filter(item => item.kind === "expense"));
  totals = { income, expense, balance: income - expense };
  for (const [key, value] of Object.entries(totals)) get(key + "-total").textContent = money(value);
  get("balance-summary").classList.toggle("negative", totals.balance < 0n);
  get("period-title").textContent = periodLabel();
  get("monthly").setAttribute("aria-pressed", String(mode === "month"));
  get("yearly").setAttribute("aria-pressed", String(mode === "year"));
  get("previous").disabled = year <= 1900 && (mode === "year" || month === 1);
  get("next").disabled = year >= 9999 && (mode === "year" || month === 12);
  if (!customDate) fields.date.value = defaultDate();
  drawChart();
  if (nativeUI) nativeUI.postMessage({
    period: periodLabel(), mode, date: fields.date.value, loadError,
    totals: Object.fromEntries(Object.entries(totals).map(([key, value]) => [key, money(value)])),
    hasPrevious: !get("previous").disabled, hasNext: !get("next").disabled,
    categories: Object.fromEntries(Object.entries(categories).map(([type, items]) =>
      [type, items.map(([id, label]) => ({id, label}))]))
  });
}

const siteSvg = "http://www.w3.org/2000/svg";
function svg(tag, attributes, label) {
  const element = document.createElementNS(siteSvg, tag);
  for (const [key, value] of Object.entries(attributes)) element.setAttribute(key, value);
  if (label !== undefined) element.textContent = label;
  return element;
}
function drawChart() {
  const chart = get("chart");
  get("chart-hint").hidden = !active.length;
  if (!active.length) {
    chart.replaceChildren(get("empty-chart").content.cloneNode(true));
    return;
  }

  const colors = {income: "#8e8e93", expense: "#eb5454", balance: "#34a853"};
  const breakdown = kind => categories[kind].map(([key, label]) => ({
    key, label, color: colors[kind], value: sum(active.filter(item => item.category === key))
  })).filter(link => link.value > 0n).sort((a, b) => a.value === b.value ? 0 : a.value > b.value ? -1 : 1);
  const links = breakdown("expense");
  if (totals.balance > 0n) links.push({key: "balance", label: "남은 금액", color: colors.balance, value: totals.balance});
  const sources = breakdown("income");
  if (totals.balance < 0n) sources.push({key: "deficit", label: "부족분", color: colors.expense, value: -totals.balance});
  const flowTotal = totals.balance < 0n ? totals.expense : totals.income;
  const width = 340;
  const height = Math.max(222, 32 * Math.max(links.length, sources.length) + 54);
  const scale = Math.min(112, height - 50);
  const graphic = svg("svg", {viewBox: "0 0 " + width + " " + height, role: "group", "aria-label": "수입과 지출 생키 다이어그램"});
  const columns = {source: 96, middle: 166, target: 242};
  const middleTop = (height - scale) / 2;
  const middleWidth = 7;

  function compact(value) {
    if (value <= 99_999_999n) return money(value);
    return new Intl.NumberFormat("ko-KR", {notation: "compact", maximumFractionDigits: 1}).format(value) + "원";
  }
  function branch(item, start, end, y1, y2, thickness, left) {
    const edge = left ? start : end;
    const edgeY = left ? y1 : y2;
    const midpoint = (start + end) / 2;
    const ramp = svg("g", {
      class: "ribbon", role: "button", tabindex: "0", "data-branch": item.key,
      "aria-label": item.label + " " + money(item.value) + " 상세 보기"
    });
    function curve(from, to) {
      return "C " + midpoint + " " + from + " " + midpoint + " " + to + " " + end + " " + to;
    }
    const top = "M " + start + " " + y1 + " " + curve(y1, y2);
    const fill = top + " L " + end + " " + (y2 + thickness) +
      " C " + midpoint + " " + (y2 + thickness) + " " + midpoint + " " + (y1 + thickness) +
      " " + start + " " + (y1 + thickness) + " Z";
    ramp.append(svg("path", {d: fill, fill: item.color, class: "flow"}));
    const center1 = y1 + thickness / 2;
    const center2 = y2 + thickness / 2;
    ramp.append(svg("path", {
      d: "M " + start + " " + center1 + " " + curve(center1, center2),
      fill: "none", stroke: "transparent", "stroke-width": "13", "pointer-events": "stroke"
    }));
    ramp.append(svg("rect", {
      x: left ? edge - 6 : edge, y: edgeY, width: 6, height: Math.max(thickness, 2),
      rx: 2, fill: item.color, class: "node"
    }));
    const anchor = left ? "end" : "start";
    const textX = left ? edge - 11 : edge + 11;
    const center = edgeY + thickness / 2;
    const textWidth = left ? edge - 9 : width - edge - 8;
    ramp.append(
      svg("rect", {
        x: left ? 0 : edge + 8, y: center - 15, width: textWidth, height: 30, fill: "transparent"
      }),
      svg("text", {x: textX, y: center - 1, "text-anchor": anchor, class: "svg-label"}, item.label),
      svg("text", {x: textX, y: center + 13, "text-anchor": anchor, class: "svg-value"}, compact(item.value))
    );
    ramp.addEventListener("click", () => openDetail(item.key));
    ramp.addEventListener("keydown", event => {
      if (event.key !== "Enter" && event.key !== " ") return;
      event.preventDefault();
      openDetail(item.key);
    });
    graphic.append(ramp);
  }

  function paint(items, left) {
    const sizes = items.map(item => scale * Number(item.value) / Number(flowTotal));
    const available = height - 16;
    const gaps = Math.min(14, (available - sizes.reduce((total, size) => total + Math.max(size, 28), 0)) / Math.max(1, items.length - 1));
    const slots = sizes.map(size => Math.max(size, 28));
    const gap = Math.max(0, gaps);
    const occupied = slots.reduce((total, size) => total + size, 0) + gap * (slots.length - 1);
    let outer = (height - occupied) / 2;
    let inner = middleTop;
    items.forEach((item, index) => {
      const size = sizes[index];
      const edge = outer + (slots[index] - size) / 2;
      if (left) branch(item, columns.source, columns.middle, edge, inner, size, true);
      else branch(item, columns.middle + middleWidth, columns.target, inner, edge, size, false);
      outer += slots[index] + gap;
      inner += size;
    });
  }
  paint(sources, true);
  paint(links, false);
  graphic.append(svg("rect", {x: columns.middle, y: middleTop, width: middleWidth, height: scale, rx: 3, fill: colors.income}));
  chart.replaceChildren(graphic);
}

function openDetail(key) {
  finishEditing();
  selectedDetail = key;
  renderDetail(true);
  if (!nativeUI && !get("detail").open) get("detail").showModal();
}
function renderDetail(present = false) {
  const key = selectedDetail;
  const balance = key === "balance" || key === "deficit";
  const entries = balance ? [] : active.filter(item => key === "income" || key === "expense"
    ? item.kind === key : item.category === key);
  entries.sort((a, b) => b.date.localeCompare(a.date));
  const amount = balance ? (key === "deficit" ? (totals.balance < 0n ? -totals.balance : 0n) : totals.balance) : sum(entries);
  const title = {
    income: "수입 내역", expense: "지출 내역", balance: "남은 금액", deficit: "부족분"
  }[key] ?? catalog.get(key)[1];
  const explanation = key === "deficit"
    ? "지출 " + money(totals.expense) + " − 수입 " + money(totals.income)
    : balance ? "수입 " + money(totals.income) + " − 지출 " + money(totals.expense)
    : entries.length ? "" : "이 기간에 해당하는 내역이 없어요.";
  if (nativeUI) {
    nativeUI.postMessage({detail: {
      present, title, period: periodLabel(), total: money(amount), explanation,
      entries: entries.map(item => ({
        ...item, amount: String(item.amount), displayAmount: money(item.amount),
        categoryLabel: catalog.get(item.category)[1]
      }))
    }});
    return;
  }
  get("detail-period").textContent = periodLabel();
  get("detail-title").textContent = title;
  get("detail-total").textContent = money(amount);
  get("detail-total").classList.toggle("negative", amount < 0n || key === "deficit");
  const note = get("detail-note");
  note.hidden = !explanation;
  note.textContent = explanation;
  get("detail-list").replaceChildren(...entries.map(item => {
    const row = document.createElement("li");
    const info = document.createElement("div");
    const name = document.createElement("strong");
    name.className = "record-name";
    name.textContent = item.name;
    const meta = document.createElement("span");
    meta.className = "record-meta";
    meta.textContent = item.date + " · " + catalog.get(item.category)[1];
    info.append(name, meta);
    const value = document.createElement("strong");
    value.className = "record-amount";
    value.textContent = money(item.amount);
    const edit = document.createElement("button");
    edit.className = "edit-record";
    edit.textContent = "수정";
    edit.setAttribute("aria-label", item.name + " 수정");
    edit.addEventListener("click", () => beginEditing(item));
    const remove = document.createElement("button");
    remove.className = "remove";
    remove.textContent = "삭제";
    remove.setAttribute("aria-label", item.name + " 삭제");
    remove.addEventListener("click", async () => {
      if (locked || !window.confirm("‘" + item.name + "’ 기록을 삭제할까요?")) return;
      const error = await deleteRecord(item.id);
      if (error) {
        note.hidden = false;
        note.textContent = error;
      }
    });
    row.append(info, value, edit, remove);
    return row;
  }));
}

const editForm = get("edit-form");
let editingRecord = null;
function beginEditing(item) {
  if (locked) return;
  editingRecord = item;
  get("edit-name").value = item.name;
  get("edit-amount").value = String(item.amount);
  get("edit-date").value = item.date;
  get("edit-category").replaceChildren(...categories[item.kind].map(([id, label]) =>
    new Option(label, id, false, id === item.category)));
  get("edit-error").hidden = true;
  get("detail-content").hidden = true;
  editForm.hidden = false;
}
function finishEditing() {
  if (!editingRecord) return;
  editingRecord = null;
  editForm.hidden = true;
  get("detail-content").hidden = false;
}
get("cancel-edit").addEventListener("click", finishEditing);
get("detail").addEventListener("close", finishEditing);
editForm.addEventListener("submit", async event => {
  event.preventDefault();
  if (!editingRecord || locked) return;
  const saveButton = get("save-edit");
  saveButton.disabled = true;
  const error = await updateRecord({
    id: editingRecord.id, name: get("edit-name").value, amount: get("edit-amount").value,
    category: get("edit-category").value, date: get("edit-date").value
  });
  saveButton.disabled = false;
  const status = get("edit-error");
  status.textContent = error;
  status.hidden = !error;
  if (!error) finishEditing();
});

async function deleteRecord(id) {
  if (!records.some(item => item.id === id)) return "해당 기록을 찾을 수 없어요.";
  if (!await save(records.filter(item => item.id !== id))) return "삭제하지 못했어요. 다시 시도해 주세요.";
  draw();
  if (selectedDetail) renderDetail();
  return "";
}

async function updateRecord(input) {
  if (locked || loadError) return "지금은 수정할 수 없어요. 잠시 후 다시 시도해 주세요.";
  const original = records.find(item => item.id === input.id);
  if (!original) return "해당 기록을 찾을 수 없어요.";
  const name = input.name.trim();
  const amount = parseAmount(input.amount);
  const {category, date} = input;
  const error = validateRecord(original.kind, name, amount, category, date);
  if (error) return error;
  if (!await save(records.map(item => item.id === original.id
    ? {...item, name, amount, category, date} : item))) return "수정 내용을 저장하지 못했어요. 다시 시도해 주세요.";
  // Show the saved record in its new period, even after a cross-year move.
  if (!date.startsWith(periodKey())) {
    year = Number(date.slice(0, 4));
    month = Number(date.slice(5, 7));
    customDate = false;
  }
  draw();
  if (selectedDetail === original.category) selectedDetail = category;
  if (selectedDetail) renderDetail();
  toast("기록을 수정했어요");
  return "";
}

get("monthly").addEventListener("click", () => setPeriodUnit("month"));
get("yearly").addEventListener("click", () => setPeriodUnit("year"));
get("previous").addEventListener("click", () => movePeriod(-1));
get("next").addEventListener("click", () => movePeriod(1));
get("expense-kind").addEventListener("click", () => setKind("expense"));
get("income-kind").addEventListener("click", () => setKind("income"));
for (const key of ["income", "expense", "balance"]) get(key + "-summary").addEventListener("click", () => openDetail(key));
get("date-toggle").addEventListener("click", () => {
  const row = get("date-row");
  row.hidden = !row.hidden;
  get("date-toggle").setAttribute("aria-expanded", String(!row.hidden));
});
fields.date.addEventListener("change", () => { customDate = true; });
fields.date.min = "1900-01-01";
fields.date.max = "9999-12-31";
get("detail").addEventListener("click", event => {
  if (event.target !== get("detail")) return;
  const bounds = get("detail").getBoundingClientRect();
  if (event.clientY < bounds.top || event.clientX < bounds.left || event.clientX > bounds.right) get("detail").close();
});
function parseAmount(text) {
  const raw = text.trim().replaceAll(",", "");
  return /^\d+$/.test(raw) ? Number(raw) : NaN;
}

function validateRecord(kind, name, amount, category, date) {
  if (!name || name.length > 80 || !Number.isSafeInteger(amount) || amount <= 0 || amount > maxAmount) {
    return "이름은 80자까지, 금액은 1원~999,999,999,999원까지 입력해 주세요.";
  }
  if (!categories[kind]?.some(item => item[0] === category)) return "카테고리를 선택해 주세요.";
  const checkDate = new Date(date + "T00:00:00Z");
  if (!/^\d{4}-\d{2}-\d{2}$/.test(date) || date < "1900-01-01" || date > "9999-12-31" ||
      !Number.isFinite(checkDate.getTime()) || checkDate.toISOString().slice(0, 10) !== date) {
    return "기록 날짜를 확인해 주세요.";
  }
  return "";
}

async function addRecord(input) {
  if (loadError) return "저장된 기록을 읽지 못했어요. 앱을 다시 열어 주세요.";
  if (locked) return "저장 중이에요. 잠시 후 다시 시도해 주세요.";
  const name = input.name.trim();
  const amount = parseAmount(input.amount);
  const {category, date, kind: recordKind} = input;
  const error = validateRecord(recordKind, name, amount, category, date);
  if (error) return error;
  const created = {
    id: Array.from(crypto.getRandomValues(new Uint8Array(16)), byte => byte.toString(16).padStart(2, "0")).join(""),
    name, amount, category, date, kind: recordKind
  };
  if (!await save([created, ...records])) return "저장하지 못했어요. 다시 시도해 주세요.";
  year = Number(date.slice(0, 4));
  month = Number(date.slice(5, 7));
  customDate = false;
  draw();
  toast(created.kind === "income" ? "수입을 추가했어요" : "지출을 추가했어요");
  return "";
}

function suggestNames(kind, text) {
  const query = text.trim().normalize("NFC").toLocaleLowerCase("ko-KR");
  if (!query) return [];
  const matches = new Map();
  for (const item of records) {
    if (item.kind !== kind) continue;
    const key = item.name.trim().normalize("NFC").toLocaleLowerCase("ko-KR");
    if (!key.includes(query) || key === query) continue;
    const previous = matches.get(key);
    if (!previous || item.date > previous.date) matches.set(key, item);
  }
  return [...matches.entries()].sort(([a, first], [b, second]) =>
    Number(b.startsWith(query)) - Number(a.startsWith(query)) ||
    second.date.localeCompare(first.date))
    .slice(0, 3).map(([, item]) => item.name);
}

window.ledgerActions = {setPeriodUnit, movePeriod, addRecord, updateRecord, deleteRecord, openDetail, suggestNames};
get("entry-form").addEventListener("submit", async event => {
  event.preventDefault();
  const error = await addRecord({
    name: fields.name.value, amount: fields.amount.value,
    category: fields.category.value, date: fields.date.value, kind
  });
  showError(error);
  if (error) return;
  fields.name.value = "";
  fields.amount.value = "";
  fields.category.value = "";
  document.activeElement?.blur();
});

setKind("expense");
draw();
if (loadError) {
  get("add-entry").disabled = true;
  showError("저장된 기록을 읽지 못했어요. 앱을 다시 열어 주세요.");
}
