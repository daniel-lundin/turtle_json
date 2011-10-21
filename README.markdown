# turtle_json
Small library for parsing and generating json.

# Usage
## Output
    JsonNode* root = JsonNode::object_node();
    root->m_array.pushback(JsonNode::string_node("ABC"));
    root->m_array.pushback(JsonNode::string_node("DEF"));

    JsonNode* obj = JsonNode::object_node();
    obj->m_dict["One"] = JsonNode::integer_node(1);
    obj->m_dict["Two"] = JsonNode::integer_node(2);
    obj->m_dict["Three"] = JsonNode::integer_node(3);
    root->m_array.pushback(obj);

    std::ofstream ofs("testout.json");
    dump(ofs, root);

Will result in:

    [
        "ABC",
        "DEF",
        {
            "One": 1,
            "Two": 2,
            "Three": 3
        }
    ]
