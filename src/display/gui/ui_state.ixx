export module synodic.soul.gui:ui_state;

import std;
import :ui_types;

// ============================================================================
// UI State Configuration System
// Designed for future C++ reflection integration (P2996, P1240)
// ============================================================================

/// Forward declaration for serialization
export class UIStateStore;

/// Concept for types that can be stored in UI state
/// When reflection lands, this will be replaced with automatic reflection
export template<typename T>
concept UISerializable = std::is_default_constructible_v<T> &&
                         std::is_copy_assignable_v<T> &&
                         (std::is_arithmetic_v<T> || 
                          std::is_same_v<T, std::string> ||
                          std::is_enum_v<T>);

// ============================================================================
// Type-erased state value for heterogeneous storage
// ============================================================================

export class UIStateValue {
public:
	UIStateValue() = default;

	template<UISerializable T>
	explicit UIStateValue(const T& value) {
		Set(value);
	}

	template<UISerializable T>
	void Set(const T& value) {
		// Store type info for future reflection
		typeHash_ = typeid(T).hash_code();
		
		if constexpr (std::is_same_v<T, std::string>) {
			stringValue_ = value;
			valueType_ = ValueType::String;
		} else if constexpr (std::is_integral_v<T>) {
			intValue_ = static_cast<std::int64_t>(value);
			valueType_ = ValueType::Integer;
		} else if constexpr (std::is_floating_point_v<T>) {
			floatValue_ = static_cast<double>(value);
			valueType_ = ValueType::Float;
		} else if constexpr (std::is_enum_v<T>) {
			intValue_ = static_cast<std::int64_t>(value);
			valueType_ = ValueType::Enum;
			// Store enum type name for future reflection-based reconstruction
			enumTypeName_ = typeid(T).name();
		}
	}

	template<UISerializable T>
	[[nodiscard]] std::optional<T> Get() const {
		if constexpr (std::is_same_v<T, std::string>) {
			if (valueType_ == ValueType::String) return stringValue_;
		} else if constexpr (std::is_integral_v<T>) {
			if (valueType_ == ValueType::Integer) return static_cast<T>(intValue_);
		} else if constexpr (std::is_floating_point_v<T>) {
			if (valueType_ == ValueType::Float) return static_cast<T>(floatValue_);
		} else if constexpr (std::is_enum_v<T>) {
			if (valueType_ == ValueType::Enum) return static_cast<T>(intValue_);
		}
		return std::nullopt;
	}

	[[nodiscard]] bool HasValue() const { return valueType_ != ValueType::None; }

	// For future glaze serialization
	[[nodiscard]] std::size_t TypeHash() const { return typeHash_; }

private:
	enum class ValueType { None, Integer, Float, String, Enum };
	
	ValueType valueType_ = ValueType::None;
	std::size_t typeHash_ = 0;
	std::int64_t intValue_ = 0;
	double floatValue_ = 0.0;
	std::string stringValue_;
	std::string enumTypeName_;  // For reflection-based enum reconstruction
};

// ============================================================================
// State binding - connects widget state to external values
// ============================================================================

export template<typename T>
class UIBinding {
public:
	using Getter = std::function<T()>;
	using Setter = std::function<void(const T&)>;

	UIBinding() = default;

	/// Create a two-way binding
	UIBinding(Getter getter, Setter setter)
		: getter_(std::move(getter)), setter_(std::move(setter)) {}

	/// Create a read-only binding
	static UIBinding ReadOnly(Getter getter) {
		return UIBinding(std::move(getter), nullptr);
	}

	/// Create a binding to a variable reference
	static UIBinding ToVariable(T& variable) {
		return UIBinding(
			[&variable]() { return variable; },
			[&variable](const T& value) { variable = value; }
		);
	}

	[[nodiscard]] T Get() const {
		return getter_ ? getter_() : T{};
	}

	void Set(const T& value) {
		if (setter_) setter_(value);
	}

	[[nodiscard]] bool IsReadOnly() const { return !setter_; }
	[[nodiscard]] bool IsValid() const { return getter_ != nullptr; }

private:
	Getter getter_;
	Setter setter_;
};

// ============================================================================
// Widget State Container
// Stores all state for a single widget
// ============================================================================

export class WidgetState {
public:
	explicit WidgetState(WidgetID id) : id_(id) {}

	[[nodiscard]] WidgetID Id() const { return id_; }

	/// Store a named value
	template<UISerializable T>
	void Set(std::string_view key, const T& value) {
		values_[std::string(key)] = UIStateValue(value);
	}

	/// Retrieve a named value
	template<UISerializable T>
	[[nodiscard]] std::optional<T> Get(std::string_view key) const {
		if (auto it = values_.find(std::string(key)); it != values_.end()) {
			return it->second.Get<T>();
		}
		return std::nullopt;
	}

	/// Check if a key exists
	[[nodiscard]] bool Has(std::string_view key) const {
		return values_.contains(std::string(key));
	}

	/// Get all keys for serialization
	[[nodiscard]] std::vector<std::string> Keys() const {
		std::vector<std::string> result;
		result.reserve(values_.size());
		for (const auto& [key, _] : values_) {
			result.push_back(key);
		}
		return result;
	}

private:
	WidgetID id_;
	std::unordered_map<std::string, UIStateValue> values_;
};

// ============================================================================
// UI State Store
// Central repository for all widget states
// Designed for serialization with glaze
// ============================================================================

export class UIStateStore {
public:
	/// Get or create state for a widget
	WidgetState& GetState(WidgetID id) {
		if (auto it = states_.find(id); it != states_.end()) {
			return it->second;
		}
		auto [it, _] = states_.emplace(id, WidgetState(id));
		return it->second;
	}

	/// Check if state exists for a widget
	[[nodiscard]] bool HasState(WidgetID id) const {
		return states_.contains(id);
	}

	/// Remove state for a widget
	void RemoveState(WidgetID id) {
		states_.erase(id);
	}

	/// Clear all state
	void Clear() {
		states_.clear();
	}

	/// Get all widget IDs with state
	[[nodiscard]] std::vector<WidgetID> GetWidgetIds() const {
		std::vector<WidgetID> result;
		result.reserve(states_.size());
		for (const auto& [id, _] : states_) {
			result.push_back(id);
		}
		return result;
	}

	// =========================================================================
	// Serialization Interface (for glaze integration)
	// When C++ reflection arrives, these can be auto-generated
	// =========================================================================

	/// Export state to JSON-compatible structure
	/// Returns: map of widget_id -> { key -> value }
	[[nodiscard]] std::string ExportToJson() const {
		// Placeholder - will integrate with glaze
		// Structure: { "widgets": { "<id>": { "<key>": <value>, ... }, ... } }
		return "{}";
	}

	/// Import state from JSON
	bool ImportFromJson(std::string_view json) {
		// Placeholder - will integrate with glaze
		(void)json;
		return false;
	}

private:
	std::unordered_map<WidgetID, WidgetState> states_;
};

// ============================================================================
// Config Schema (for future reflection-based UI generation)
// ============================================================================

/// Describes a single configurable field
export struct UIConfigField {
	std::string name;
	std::string displayName;
	std::string description;
	std::string typeName;        // For reflection: "int", "float", "enum:MyEnum"
	UIStateValue defaultValue;
	UIStateValue minValue;       // For numeric types
	UIStateValue maxValue;
	std::vector<std::string> enumOptions;  // For enum types
};

/// Describes a group of related config fields
export struct UIConfigGroup {
	std::string name;
	std::string displayName;
	std::vector<UIConfigField> fields;
};

/// Full config schema - describes all configurable aspects of a system
/// When reflection lands, this will be auto-generated from structs with attributes
export struct UIConfigSchema {
	std::string name;
	std::string version;
	std::vector<UIConfigGroup> groups;
};
